#include "proto/proto.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/platform/RuntimeSettings.hh"
#include "proto/core/asset-system/serialization.hh"
#include "proto/core/graphics/ShaderProgram.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/entity-system/interface.hh"
#include "proto/core/graphics/rendering.hh"
#include "proto/core/graphics/Camera.hh"
#include "proto/core/meta.hh"

using namespace proto;

PROTO_SETUP { // (RuntimeSettings * settings)
}

AssetHandle prev_shader_h;
ShaderProgram * prev_shader;
Entity e;

PROTO_INIT {
    vardump(1);
    auto& ctx = *proto::context;
    Array<int> testarr;
    testarr.init(0, &ctx.memory);
    #if 0 
    serialization::load_asset_dir("outmesh/");

    #if 0
    prev_shader_h = create_asset<ShaderProgram>("simple-shader");
    prev_shader = get_asset<ShaderProgram>(prev_shader_h);

    prev_shader->attach_shader_file(ShaderType::Vert, "prev_vert.glsl");
    prev_shader->attach_shader_file(ShaderType::Frag, "prev_frag.glsl");
    prev_shader->link();
    #endif

    for(auto& m : ctx.meshes) graphics::gpu_upload(&m);

    ctx.camera.fov = (float)M_PI * 2.0f / 3.0f;
    ctx.camera.aspect = (float)ctx.window_size.x/ctx.window_size.y;
    ctx.camera.near = 0.01f;
    ctx.camera.far = 100.f;

    vardump(1);
    e = create_entity();
    add_component<TransformComp>(e);
    add_component<RenderMeshComp>(e).mesh = ctx.meshes[1].handle;
    #endif
}

PROTO_UPDATE {
    #if 0
    auto& ctx = *proto::context;
    float& time = ctx.clock.elapsed_time;
    vardump(1);

    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //get_asset_ref<ShaderProgram>(ctx.quad_shader_h).use();

    //graphics::render_quad();
    ///graphics::render_scene();
    #endif
}

PROTO_LINK {}

PROTO_UNLINK {}

PROTO_CLOSE {}
