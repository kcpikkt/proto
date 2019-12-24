#include "proto/proto.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/asset-system/serialization.hh"
#include "proto/core/graphics.hh" 
#include "proto/core/util/namespace-shorthands.hh" 
#include "proto/core/platform/File.hh" 
using namespace proto;

AssetHandle mytex;

PROTO_INIT {
    mytex = ser::load_asset("outmesh/vase_round_Texture2D.past");
    gfx::gpu_upload(get_asset<Texture2D>(mytex));
    if(!mytex) debug_info(0, "hwoa");
}

PROTO_UPDATE {
    auto& ctx = *proto::context;

    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gfx::render_texture_quad(gfx::bind_texture(mytex));

    gfx::stale_all_texture_slots();
}
