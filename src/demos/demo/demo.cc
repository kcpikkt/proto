#include "proto/proto.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/platform/RuntimeSettings.hh"
#include "proto/core/asset-system/serialization.hh"
#include "proto/core/graphics/ShaderProgram.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/entity-system/interface.hh"
#include "proto/core/graphics/rendering.hh"
#include "proto/core/graphics/Camera.hh"
#include "proto/core/graphics/Framebuffer.hh"
#include "proto/core/graphics/Renderbuffer.hh"
#include "proto/core/util/namespace-shorthands.hh"
#include "proto/core/meta.hh"
#include "proto/core/math/random.hh"
#include "proto/core/math/common.hh"

using namespace proto;

PROTO_SETUP { // (RuntimeSettings * settings)
}

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

u8 client_data_begin; //////////////////////////////

AssetHandle prev_shader_h;
ShaderProgram * prev_shader;

Framebuffer dirlight_shadowmap_buf;
AssetHandle dirlight_shadowmap_shader_h;
AssetHandle dirlight_shadowmap_h;


ivec2 dirlight_shadowmap_size = vec2(4096);
float dirlight_near = 1.1f, dirlight_far = 100.0f;

vec3 sun_dir   = vec3( 0.3,-1.0,  -0.5);
vec3 sun_pos   = vec3( 0.0, 400.0, 0.0);
vec3 sun_color = vec3( 1.0, 1.0,   1.0);

// gbuffer
Framebuffer gbuffer;
AssetHandle gbuf_position_h;
AssetHandle gbuf_normal_h;
AssetHandle gbuf_albedo_spec_h;
AssetHandle gbuf_depth_h;
//Renderbuffer gbuf_depth;

Framebuffer prebuffer;
AssetHandle prebuf_ssao_h;
AssetHandle prebuf_pingpong_h;

AssetHandle ssao_shader_h;
AssetHandle ssao_noise_h;
Array<vec3> ssao_kernel;
ivec2 ssao_noise_size = ivec2(64,64);

Framebuffer defbuffer;
AssetHandle defbuf_ping_h;
AssetHandle defbuf_bloom_h;
AssetHandle defbuf_pong_h;

Framebuffer postbuffer;
AssetHandle postbuf_ping_h;
AssetHandle postbuf_pong_h;

//shaders
AssetHandle deffered_shader_h;
AssetHandle ssao_gaussian_shader_h;
AssetHandle skybox_shader_h;
AssetHandle hdr_shader_h;

AssetHandle skybox_h;
AssetHandle skyboxa_h;

Entity sponza;
Entity teapot_0;
Entity teapot_1;
Entity teapot_2;

u8 client_data_end; 

u32 skybox_cubemap_gl_id;

char randomdata[1024];

void mouse_move_callback(MouseMoveEvent& ev) {
    float factor = 0.01 * context->clock.delta_time;

    context->camera.rotation = 
        glm::rotate(context->camera.rotation,
                    factor * ev.delta.x, vec3(0.0, 1.0, 0.0));

} MouseMoveInputSink mouse_move_input_sink;


PROTO_INIT {
    auto& ctx = *proto::context;

    mouse_move_input_sink.init(ctx.mouse_move_input_channel, mouse_move_callback);

    ctx.camera.position = vec3(0.0,1.65,10.0);
    ctx.camera.rotation = angle_axis(0.0, vec3(0.0,1.0,0.0));
    ctx.camera.fov = (float)M_PI * 1.4f / 3.0f;
    ctx.camera.aspect = (float)ctx.window_size.x/ctx.window_size.y;
    ctx.camera.near = 0.01f;
    ctx.camera.far = 300.0f;

    serialization::load_asset_dir("outmesh/");
    skybox_h = serialization::load_asset("res/cubemap.past");

    gfx::gpu_upload(get_asset<Cubemap>(skybox_h));

    sponza = create_entity();

    auto& transform = add_component<TransformComp>(sponza);
    transform.position = vec3(0.0, 0.0, 0.0);
    transform.rotation = angle_axis(M_PI/2.0f, vec3(0.0, 1.0, 0.0));
    transform.scale = vec3(0.0085); // sponza has around 11.65 meters

    if(Mesh * sponza_mesh = get_asset<Mesh>( make_handle<Mesh>("sponza") )) {
        add_component<RenderMeshComp>(sponza).mesh = make_handle<Mesh>("sponza");

    } else
        debug_info(debug::category::main, "Failed to assign mesh to entity.");


    teapot_0 = create_entity();
    //teapot_1 = create_entity();
    //teapot_2 = create_entity();

    {
    auto& transform = add_component<TransformComp>(teapot_0);
    transform.position = vec3(0.0, 1.0, 0.0);
    transform.scale = vec3(0.085);

    if(Mesh * teapot_mesh = get_asset<Mesh>( make_handle<Mesh>("teapot") )) {
        add_component<RenderMeshComp>(teapot_0).mesh = make_handle<Mesh>("teapot");
        teapot_mesh->spans[0].material.specular_color = vec3(8.0);

    } else
        debug_info(debug::category::main, "Failed to assign mesh to entity.");
    }




    for(auto& t : ctx.textures) gfx::gpu_upload(&t);
    for(auto& m : ctx.meshes) gfx::gpu_upload(&m);
    auto& scr_size = context->window_size;
    
// render to texture config, we obviously don't want mipmaps on them
    auto no_mipmap =
        [](Texture2D& tex){ tex.flags.unset(Texture2D::mipmap_bit); };

    auto& dirlight_shadowmap =
        create_asset_rref<Texture2D>("dirlight_shadowmap_texture")
            .$_configure(no_mipmap)
            .$_init(dirlight_shadowmap_size, GL_DEPTH_COMPONENT24,
                    GL_DEPTH_COMPONENT, GL_FLOAT);
    dirlight_shadowmap_h = dirlight_shadowmap.handle;

    dirlight_shadowmap_buf
        .$_init(dirlight_shadowmap_size, 0)
        .$_bind()
        .$_add_depth_attachment(dirlight_shadowmap)
        .finalize();

    dirlight_shadowmap_shader_h = 
        create_init_asset_rref<ShaderProgram>("dirlight_shadowmap_shader")
            .$_attach_shader_file(ShaderType::Vert, "pass_mvp_vert.glsl")
            .$_attach_shader_file(ShaderType::Frag, "noop_frag.glsl")
            .$_link().handle;

// g-buffer setup
    auto& gbuf_position =
        create_asset_rref<Texture2D>("gbuffer_position_texture")
            .$_configure(no_mipmap)
            .$_init(scr_size, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    gbuf_position_h = gbuf_position.handle;

    auto& gbuf_normal = 
        create_asset_rref<Texture2D>("gbuffer_normal_texture")
            .$_configure(no_mipmap)
            .$_init(scr_size, GL_RGBA16F, GL_RGBA, GL_FLOAT);
    gbuf_normal_h = gbuf_normal.handle;

    auto& gbuf_albedo_spec =
        create_asset_rref<Texture2D>("gbuffer_albedo_spec_texture")
            .$_configure(no_mipmap)
            .$_init(scr_size, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
    gbuf_albedo_spec_h = gbuf_albedo_spec.handle;

    auto& gbuf_depth =
        create_asset_rref<Texture2D>("gbuffer_depth_texture")
            .$_configure(no_mipmap)
            .$_init(scr_size, GL_DEPTH_COMPONENT24,
                    GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE);
    gbuf_depth_h = gbuf_depth.handle;

    gbuffer
        .$_init(scr_size, 3)
        .$_bind()
        .$_add_color_attachment(gbuf_position)
        .$_add_color_attachment(gbuf_normal)
        .$_add_color_attachment(gbuf_albedo_spec)
        .$_add_depth_attachment(gbuf_depth)
        .finalize();

// ssao prebuffer setup
    ssao_kernel.init_resize(64, &ctx.memory);
    float distrib_bias = 1.5;

    for(u32 i=0; i<ssao_kernel.size(); i++)
        ssao_kernel[i] =
            glm::normalize(vec3(random::randf01() * 2.0f - 1.0f,
                                random::randf01() * 2.0f - 1.0f,
                                random::randf01() )) *
            lerp(0.11f, 1.0f, pow(random::randf01(), distrib_bias));

    Array<vec3> ssao_noise_data;
    ssao_noise_data.init_resize(ssao_noise_size.x * ssao_noise_size.y, &ctx.memory);

    for(auto& pix : ssao_noise_data)
        pix = vec3(random::randf01() * 2.0f - 1.0f,
                   random::randf01() * 2.0f - 1.0f,
                   0.0);

    ssao_noise_h =
        create_asset_rref<Texture2D>("ssao_noise_texture")
            .$_configure(no_mipmap)
            .$_init(ssao_noise_data.raw(), ssao_noise_size,
                    GL_RGB32F, GL_RGB, GL_FLOAT)
            .$_upload()
            .$_configure([](Texture2D& tex){ tex.data = nullptr; })
            .handle;


    auto ssao_tex_data_config =
        [](Texture2D& tex) {
            tex.format = GL_RED;
            tex.gpu_format = GL_RED;
            tex.size = context->window_size;
        };

    auto ssao_tex_param_config =
        []() {
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        };

    auto& prebuf_ssao =
        create_asset_rref<Texture2D>("prebuffer_ssao_texture")
            .$_configure(no_mipmap)
            .$_init(ssao_tex_param_config)
            .$_configure(ssao_tex_data_config);
    prebuf_ssao_h = prebuf_ssao.handle;

    auto& prebuf_pingpong =
        create_asset_rref<Texture2D>("prebuffer_pingpong_texture")
            .$_configure(no_mipmap)
            .$_init(ssao_tex_param_config)
            .$_configure(ssao_tex_data_config);
    prebuf_pingpong_h = prebuf_pingpong.handle;

    prebuffer
        .$_init(scr_size, 2)
        .$_bind()
        .$_add_color_attachment(prebuf_ssao)
        .$_add_color_attachment(prebuf_pingpong)
        .finalize();

    gfx::reset_framebuffer();

    ssao_shader_h = 
        create_init_asset_rref<ShaderProgram>("ssao_shader")
            .$_attach_shader_file(ShaderType::Vert, "pass_vert.glsl")
            .$_attach_shader_file(ShaderType::Frag, "ssao_frag.glsl")
            .$_link().handle;

    ssao_gaussian_shader_h = 
        create_init_asset_rref<ShaderProgram>("ssao_gaussian_shader")
            .$_attach_shader_file(ShaderType::Vert, "pass_vert.glsl")
            .$_attach_shader_file(ShaderType::Frag,
                                  "twopass_ssao_gaussian_frag.glsl")
            .$_link().handle;

    auto& defbuf_ping =
        create_asset_rref<Texture2D>("defbuffer_ping_texture")
            .$_configure(no_mipmap)
            .$_init(scr_size, GL_RGBA, GL_RGBA);
    defbuf_ping_h = defbuf_ping.handle;

    auto& defbuf_bloom =
        create_asset_rref<Texture2D>("defbuffer_bloom_texture")
            .$_configure(no_mipmap)
            .$_init(scr_size, GL_RGB, GL_RGB);
    defbuf_bloom_h = defbuf_bloom.handle;

    auto& defbuf_pong =
        create_asset_rref<Texture2D>("defbuffer_pong_texture")
            .$_configure(no_mipmap)
            .$_init(scr_size, GL_RGBA, GL_RGBA);
    defbuf_pong_h = defbuf_pong.handle;

    defbuffer
        .$_init(scr_size, 3)
        .$_bind()
        .$_add_color_attachment(defbuf_ping)
        .$_add_color_attachment(defbuf_pong)
        .$_add_color_attachment(defbuf_bloom)
        .finalize();

    deffered_shader_h = 
        create_init_asset_rref<ShaderProgram>("deferred_shader")
            .$_attach_shader_file(ShaderType::Vert, "pass_vert.glsl")
            .$_attach_shader_file(ShaderType::Frag, "deferred_frag.glsl")
            .$_link().handle;

    auto& postbuf_ping =
        create_asset_rref<Texture2D>("postbuffer_ping_texture")
            .$_configure(no_mipmap)
            .$_init(scr_size, GL_RGBA, GL_RGBA);
    postbuf_ping_h = postbuf_ping.handle;

    auto& postbuf_pong =
        create_asset_rref<Texture2D>("postbuffer_pong_texture")
            .$_configure(no_mipmap)
            .$_init(scr_size, GL_RGBA, GL_RGBA);
    postbuf_pong_h = postbuf_pong.handle;

    postbuffer
        .$_init(scr_size, 2)
        .$_bind()
        .$_add_color_attachment(postbuf_ping)
        .$_add_color_attachment(postbuf_pong)
        .finalize();


    hdr_shader_h =
        create_init_asset_rref<ShaderProgram>("hdr_shader")
            .$_attach_shader_file(ShaderType::Vert, "pass_vert.glsl")
            .$_attach_shader_file(ShaderType::Frag, "hdr_frag.glsl")
            .$_link().handle;


    gfx::stale_all_texture_slots();
    vardump(gfx::error_message());
}

////////////////////////////////////////////////////////////////////////////

#define TAKE 0

PROTO_UPDATE {
    auto& ctx = *proto::context;
    float& time = ctx.clock.elapsed_time;
    glDisable(GL_BLEND);

    #if TAKE == 0
    ctx.camera.position = vec3(0.0, 1.25, 2.0);
    #elif TAKE == 1
    ctx.camera.rotation =
        angle_axis(sin(time/25.0), glm::normalize(vec3(1.0,-1.0,0.0)));
    ctx.camera.position = vec3(1.0 * cos(time/10.0),0.65, 0.0);
    #elif TAKE == 2
    float stime = time/10.0;
    ctx.camera.position = vec3(3.0 + cos(stime)*stime * 1.2,
                               4.65,
                               -3.5 - sin(time/10.0) * 7.0);
    ctx.camera.rotation =
        angle_axis(sin(time/10.0) + 1.4, vec3(0.0,1.0,0.0));
    #elif TAKE == 3
    float stime = time/10.0;
    ctx.camera.position = vec3(3.2, 1.65 + cos(stime*2.0)/3.0
                               ,5.0 + sin(stime) * 7.0);
    #endif 

    gfx::reset_framebuffer();

    glClearColor(0.1f,0.1f,0.1f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);

    gfx::render_skybox(gfx::bind_texture(skybox_h));

    // shadows
    #if 1
    gfx::bind_framebuffer(dirlight_shadowmap_buf);
    glViewport(0, 0, dirlight_shadowmap_buf.size.x,
                     dirlight_shadowmap_buf.size.y);
    glClear(GL_DEPTH_BUFFER_BIT);

    float tmp = 17.0;
    mat4 dirlight_proj =
        glm::ortho(-tmp, tmp, -tmp, tmp,
                   dirlight_near, dirlight_far);  

    sun_pos = -sun_dir * 40.0f;
    mat4 dirlight_view =
        glm::lookAt(sun_pos, sun_pos + sun_dir, 
                    vec3( 0.0f, 1.0f, 0.0f)); 

    mat4 dirlight_vp = dirlight_proj * dirlight_view;
    for(auto& comp : context->comp.render_mesh) {
        TransformComp * transform = get_component<TransformComp>(comp.entity);
        assert(transform);
        mat4 model = transform->model();

        Mesh * mesh = get_asset<Mesh>(comp.mesh);

        if(!mesh) {
            debug_error(1, "abandoned component :/"); continue;
        }

        mat4 dirlight_mvp = dirlight_vp * model;

        get_asset_ref<ShaderProgram>(dirlight_shadowmap_shader_h)
            .$_use()
            .$_set_mat4  ("u_mvp", &dirlight_mvp);

        gfx::render_mesh(mesh, true);
    }

    #endif
    // gbuf
    gfx::bind_framebuffer(gbuffer);
    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gfx::render_gbuffer();

    gfx::bind_framebuffer(prebuffer);
    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glDisable(GL_DEPTH_TEST);

    mat4 projection = ctx.camera.projection();
    mat4 view = ctx.camera.view();
    mat4 ssao_mvp = projection * view;

    vec2 scr_size = (vec2)ctx.window_size;
    auto ssao_shader =
        get_asset_ref<ShaderProgram>(ssao_shader_h)
            .$_use()
            .$_set_mat4  ("u_mvp",            &ssao_mvp)
            .$_set_float ("u_time",           time)
            .$_set_vec2  ("u_resolution",     &scr_size)
            .$_set_float ("u_radius",         1.0f)
            .$_set_mat4  ("u_projection",     &projection)
            .$_set_mat4  ("u_view",           &view)
            .$_set_tex2D ("gbuf.position",    gbuf_position_h)
            .$_set_tex2D ("gbuf.normal",      gbuf_normal_h)
            .$_set_tex2D ("gbuf.depth",       gbuf_depth_h)
            .$_set_tex2D ("u_noise",          ssao_noise_h);

    char uniform_name[128];
    for(u32 i=0; i<ssao_kernel.size(); i++) {
        sprint(uniform_name, 128, "kernel[", i, ']');
        ssao_shader.set_vec3(uniform_name, &ssao_kernel[i]);
    }
    gfx::render_quad();

    s32 blur = 3;
    #if 1

    auto& ssao_gaussian_shader = 
        get_asset_ref<ShaderProgram>(ssao_gaussian_shader_h);
    
    glDrawBuffer(GL_COLOR_ATTACHMENT1);
    ssao_gaussian_shader
        .$_use()
        .$_set_int   ("u_mode",           0)
        .$_set_float ("u_time",           time)
        .$_set_int   ("u_size",           blur)
        .$_set_float ("u_cam_far",        ctx.camera.far)
        .$_set_float ("u_cam_near",       ctx.camera.near)
        .$_set_mat4  ("u_projection",     &projection)
        .$_set_vec2  ("u_resolution",     &scr_size)
        .$_set_tex2D ("u_depth",          gbuf_depth_h)
        .$_set_tex2D ("u_tex",            prebuf_ssao_h);
    gfx::render_quad();

    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    ssao_gaussian_shader
        .$_use()
        .$_set_int   ("u_mode",           1)
        .$_set_tex2D ("u_tex",            prebuf_pingpong_h);
    gfx::render_quad();
    #endif

    gfx::bind_framebuffer(defbuffer);

    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    auto deffered_shader =
        get_asset_ref<ShaderProgram>(deffered_shader_h)
            .$_use()
            .$_set_float ("u_time",           time)
            .$_set_int   ("u_size",           blur)
            .$_set_float ("u_cam_far",        ctx.camera.far)
            .$_set_float ("u_cam_near",       ctx.camera.near)
            .$_set_vec3  ("u_cam_pos",        &ctx.camera.position)
            .$_set_mat4  ("u_projection",     &projection)
            .$_set_mat4  ("u_view",           &view)
            .$_set_vec2  ("u_resolution",     &scr_size)
            .$_set_tex2D ("gbuf.position",    gbuf_position_h)
            .$_set_tex2D ("gbuf.normal",      gbuf_normal_h)
            .$_set_tex2D ("gbuf.albedo_spec", gbuf_albedo_spec_h)
            .$_set_tex2D ("gbuf.depth",       gbuf_depth_h)
            .$_set_tex2D ("gbuf.ssao",        prebuf_ssao_h)
            .$_set_tex2D ("u_dirlight[0].shadow_map", dirlight_shadowmap_h)
            .$_set_mat4  ("u_dirlight[0].matrix",    &dirlight_vp)
            .$_set_float ("u_dirlight[0].far",       dirlight_far)
            .$_set_vec3  ("u_dirlight[0].direction", &sun_dir)
            .$_set_vec3  ("u_dirlight[0].color",     &sun_color)
            .$_set_float ("u_dirlight[0].intensity", 0.8f);


    gfx::render_quad();

    // POSTPROCESS
    gfx::bind_framebuffer(postbuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    get_asset_ref<ShaderProgram>(hdr_shader_h)
        .$_use()
        .$_set_float ("u_time",           time)
        .$_set_vec2  ("u_resolution",     &scr_size)
        .$_set_tex2D ("gbuf.normal",      gbuf_normal_h)
        .$_set_tex2D ("u_tex",            gfx::bind_texture(defbuf_ping_h));

    gfx::render_quad();

    gfx::reset_framebuffer();

    get_asset_ref<ShaderProgram>(ctx.quad_shader_h).use();
    vec2 halfscr = (vec2)ctx.window_size/2.0f;

    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);

    #if 0
    gfx::render_texture_quad(gfx::bind_texture(gbuf_normal_h),
                             vec2(0.0), halfscr);

    gfx::render_texture_quad(gfx::bind_texture(gbuf_normal_h),
                             vec2(0.0, halfscr.y), halfscr);

    gfx::render_texture_quad(gfx::bind_texture(gbuf_normal_h),
                             vec2(halfscr.x, 0.0), halfscr);

    gfx::render_texture_quad(gfx::bind_texture(gbuf_position_h),
                             halfscr, halfscr);

    #else 
    glEnable(GL_BLEND);
    gfx::render_texture_quad(gfx::bind_texture(postbuf_ping_h),
                             vec2(0.0), 2.0f*halfscr);
    #endif
    gfx::render_std_basis();

    glEnable(GL_DEPTH_TEST);

    gfx::stale_all_texture_slots();
}

#if 0 // framebuffers as handles
PROTO_UNLINK {
    // yeah, thats hacky
    u64 preserved_data_size = &client_data_end - &client_data_begin;
    void * preserved = context->memory.alloc(preserved_data_size);

    memcpy(preserved, &client_data_begin, preserved_data_size);

    (*context->client_preserved) = preserved;
    context->client_preserved_size = preserved_data_size;
}

PROTO_LINK {
    if(*context->client_preserved) {
        u64 preserved_data_size = &client_data_end - &client_data_begin;
    
        if(preserved_data_size != context->client_preserved_size) {
            debug_warn(debug::category::main,
                       "Client data size changed between links, "
                       "cannot retrieve preservedd data.");
            return;
        }

        memcpy(&client_data_begin,
               (*context->client_preserved),
               preserved_data_size);

        (*context->client_preserved) = nullptr;
        context->client_preserved_size = 0;
    }
}
#endif

PROTO_CLOSE {}
