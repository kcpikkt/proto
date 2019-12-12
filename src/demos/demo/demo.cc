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

template<typename T, typename Return = T>
meta::conditional_t<meta::is_same_v<T, int>, T, T&> function() {
    if constexpr (meta::is_same_v<T, int>) {
        T ok;
        return ok;
    } else {
        T ok;
        return ok;
    }
}

PROTO_SETUP { // (RuntimeSettings * settings)
}

AssetHandle prev_shader_h;
ShaderProgram * prev_shader;
Entity e;
Texture2D gbuf_position;
Texture2D gbuf_normal;
Texture2D gbuf_albedo;
Framebuffer gbuffer;

PROTO_INIT {
    auto& ctx = *proto::context;

    ctx.camera.fov = (float)M_PI * 2.0f / 3.0f;
    ctx.camera.aspect = (float)ctx.window_size.x/ctx.window_size.y;
    ctx.camera.near = 0.01f;
    ctx.camera.far = 100.f;

    serialization::load_asset_dir("outmesh/");
    #if 0

    auto& scr_size = context->window_size;
    Framebuffer gbuffer;
    gbuf_position.init(scr_size, GL_RGB16F, GL_RGB);
    gbuf_normal.init(scr_size, GL_RGB16F, GL_RGB);
    gbuf_albedo.init(scr_size, GL_RGBA, GL_RGB);

    gfx::bind_framebuffer(&gbuffer);
    gbuffer.init(scr_size, 3);
    gbuffer.add_color_attachment(&gbuf_position);
    gbuffer.add_color_attachment(&gbuf_normal);
    gbuffer.add_color_attachment(&gbuf_albedo);
    gbuffer.finalize();

    u32 gbuf_depth;
    glGenRenderbuffers(1, &gbuf_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, gbuf_depth);
    glRenderbufferStorage
        (GL_RENDERBUFFER, GL_DEPTH_COMPONENT, scr_size.x, scr_size.y);
    glFramebufferRenderbuffer
        (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gbuf_depth);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        debug_warn(debug::category::graphics,
                   "Incomplete framebuffer");
    }

    e = create_entity();
    add_component<TransformComp>(e);
    add_component<RenderMeshComp>(e).mesh = ctx.meshes[1].handle;
    #endif
}

PROTO_UPDATE {
    #if 0
    auto& ctx = *proto::context;
    float& time = ctx.clock.elapsed_time;

    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //get_asset_ref<ShaderProgram>(ctx.quad_shader_h).use();
    vec2 halfscr = (vec2)ctx.window_size/2.0f;

    gfx::render_quad(gfx::bind_texture(ctx.default_checkerboard_texture_h),
                     vec2(0.0), halfscr);
    gfx::render_quad(gfx::bind_texture(ctx.default_checkerboard_texture_h),
                     vec2(0.0, halfscr.y), halfscr);
    gfx::render_quad(gfx::bind_texture(ctx.default_checkerboard_texture_h),
                     halfscr, halfscr);
    ///graphics::render_scene();
    #endif
}

PROTO_LINK {}

PROTO_UNLINK {}

PROTO_CLOSE {}
