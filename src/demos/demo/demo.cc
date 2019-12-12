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
#include "proto/core/util/namespace-shorthands.hh"
#include "proto/core/meta.hh"

using namespace proto;

PROTO_SETUP { // (RuntimeSettings * settings)
}

AssetHandle prev_shader_h;
ShaderProgram * prev_shader;
Entity e;
AssetHandle gbuf_position_h;
AssetHandle gbuf_normal_h;
AssetHandle gbuf_albedo_h;
Framebuffer gbuffer;


PROTO_INIT {
    auto& ctx = *proto::context;

    ctx.camera.fov = (float)M_PI * 2.0f / 3.0f;
    ctx.camera.aspect = (float)ctx.window_size.x/ctx.window_size.y;
    ctx.camera.near = 0.01f;
    ctx.camera.far = 100.f;

    serialization::load_asset_dir("outmesh/");

    for(auto& t : ctx.textures) {
        vardump(get_metadata(t.handle)->name);
        //gfx::gpu_upload(&t);
    }
    
    for(auto& m : ctx.meshes) gfx::gpu_upload(&m);
    auto& scr_size = context->window_size;

    auto& gbuf_position =
        create_asset_rref<Texture2D>("gbuffer_position_texture")
            .$_init(scr_size, GL_RGB16F, GL_RGB);
    gbuf_position_h = gbuf_position.handle;

    auto& gbuf_normal = 
        create_asset_rref<Texture2D>("gbuffer_normal_texture")
            .$_init(scr_size, GL_RGB16F, GL_RGB);
    gbuf_normal_h = gbuf_normal.handle;

    auto& gbuf_albedo =
        create_asset_rref<Texture2D>("gbuffer_albedo_texture")
            .$_init(scr_size, GL_RGBA, GL_RGBA);
    gbuf_albedo_h = gbuf_albedo.handle;

    gbuffer.init(scr_size, 3);
    gfx::bind_framebuffer(gbuffer);

    gbuffer
        .$_add_color_attachment(&gbuf_position)
        .$_add_color_attachment(&gbuf_normal)
        .$_add_color_attachment(&gbuf_albedo);
    gbuffer.finalize();

    u32 gbuf_depth;
    glGenRenderbuffers(1, &gbuf_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, gbuf_depth);
    glRenderbufferStorage
        (GL_RENDERBUFFER, GL_DEPTH_COMPONENT, scr_size.x, scr_size.y);
    glFramebufferRenderbuffer
        (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gbuf_depth);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        debug_warn(debug::category::graphics, "Incomplete framebuffer");
    
    vardump(gfx::error_message());
}

PROTO_UPDATE {

    auto& ctx = *proto::context;
    float& time = ctx.clock.elapsed_time;
    ctx.camera.position = vec3(0.0,0.0,0.0);

    gfx::bind_framebuffer(gbuffer);
    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gfx::render_gbuffer();
    gfx::reset_framebuffer();

    get_asset_ref<ShaderProgram>(ctx.quad_shader_h).use();
    vec2 halfscr = (vec2)ctx.window_size/2.0f;

    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gfx::render_quad(gfx::bind_texture(gbuf_position_h),
                     vec2(0.0), halfscr);
    gfx::render_quad(gfx::bind_texture(gbuf_normal_h),
                     vec2(0.0, halfscr.y), halfscr);
    gfx::render_quad(gfx::bind_texture(gbuf_albedo_h),
                     halfscr, halfscr);
    gfx::render_quad(gfx::bind_texture(ctx.default_checkerboard_texture_h),
                     vec2(halfscr.x, 0.0), halfscr);
}

PROTO_LINK {}

PROTO_UNLINK {}

PROTO_CLOSE {}
