#include "proto/proto.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/asset-system/serialization.hh"
#include "proto/core/graphics.hh" 
#include "proto/core/util/namespace-shorthands.hh" 
#include "proto/core/platform/File.hh" 
using namespace proto;

AssetHandle mytex;

PROTO_INIT {
    //mytex = ser::load_asset("vase_round_Texture2D.past");
}

//PROTO_UPDATE {
//    gfx::render_texture_quad(gfx::bind_texture(mytex));
//}
