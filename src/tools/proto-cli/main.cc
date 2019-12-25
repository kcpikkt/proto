#include "proto/proto.hh"
#include "proto/core/platform/common.hh" 
#include "proto/core/graphics/gl.hh" 
#include "proto/core/graphics/rendering.hh" 
#include "proto/core/util/namespace-shorthands.hh" 
#include "proto/core/asset-system/interface.hh" 
#include "proto/core/asset-system/serialization.hh" 
#include "proto/core/util/argparse.hh" 
#include "proto/core/util/String.hh" 

#include "assimp/Importer.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "common.hh"
//#include "fetch_mesh.hh"
#include "fetch_texture.hh"
#include "fetch_cubemap.hh"

using namespace proto;

PROTO_SETUP {
    settings->mode = RuntimeSettings::terminal_mode_bit;
    settings->asset_paths = "res/";
}
#define LEN(arr) (sizeof(arr)/sizeof(arr[0]))

#define FAIL(...) { \
    log_error(debug::category::main, __VA_ARGS__); return;}

StringView cmdline_sentences[] = {"parse model",
                                  "parse cubemap",
                                  "parse texture",
};
PROTO_INIT {
    assert(proto::context);
    auto& ctx = *proto::context;
    search_paths.init_split(".", ':', &ctx.memory);

    StringView sentence =
        argparse::match_sentence(cmdline_sentences, LEN(cmdline_sentences), 2);

    if(sentence) {
        //////////////////////////////////////////////////////////////////////////////////////////////////
        /*  */ if(strview_cmp(sentence, "parse texture")) {
            StringView image_file = ctx.argv[4];
            StringView output_file = ctx.argv[5];

            if(ctx.argc != 6) {
                log_error(debug::category::main, "parse texture [image file] [output file]");
                return;
            }

            String final_image_file = sys::search_for_file(image_file, search_paths);

            if(!final_image_file)
                FAIL("Could not find ", image_file);

            AssetHandle handle = fetch_texture(final_image_file.view());

            if(!handle) {
                FAIL("Failed to fetch texture ", final_image_file.view())}

            if(sys::is_file(output_file))
                log_warn(debug::category::main, "overwriting ", output_file);

            if(ser::save_asset(get_asset<Texture2D>(handle), output_file)) {
                FAIL("Could not save texture to ", output_file);
            } else {
                log_info(debug::category::main, "Texture written to file ", output_file);
            }
            return;

        //////////////////////////////////////////////////////////////////////////////////////////////////
        } else if(strview_cmp(sentence, "parse cubemap")) {
            if(ctx.argc != 11) {
                log_error(debug::category::main,
                          "parse cubemap [right] [left] [up] [down] [forward] [back] [output file]");
                return;
            }

            StringView output_file = ctx.argv[10];

            String right   = sys::search_for_file(ctx.argv[4], search_paths);
            String left    = sys::search_for_file(ctx.argv[5], search_paths);
            String up      = sys::search_for_file(ctx.argv[6], search_paths);
            String down    = sys::search_for_file(ctx.argv[7], search_paths);
            String forward = sys::search_for_file(ctx.argv[8], search_paths);
            String back    = sys::search_for_file(ctx.argv[9], search_paths);

            auto try_fail = [](bool cond, StringView filepath) {
                                if(!cond) {
                                    log_error(debug::category::data,
                                              "Could not find ", filepath);
                                    return 1;
                                }else
                                    return 0;};

            if(try_fail(right   , ctx.argv[4]) ||
               try_fail(left    , ctx.argv[5]) ||
               try_fail(up      , ctx.argv[6]) ||
               try_fail(down    , ctx.argv[7]) ||
               try_fail(forward , ctx.argv[8]) ||
               try_fail(back    , ctx.argv[9]) )
            {
                log_info(debug::category::data, "Could not create cubemap");
                return;
            }

            AssetHandle handle = fetch_cubemap(right, left, up, down, forward, back);

            if(!handle) {
                FAIL("Failed to fetch cubemap")}

            if(sys::is_file(output_file))
                log_warn(debug::category::main, "overwriting ", output_file);

            if(ser::save_asset(get_asset<Cubemap>(handle), output_file)) {
                FAIL("Could not save cubemap to ", output_file);
            } else {
                log_info(debug::category::main, "Cubemap written to file ", output_file);
            }
            return;

        //////////////////////////////////////////////////////////////////////////////////////////////////
        } else if(strview_cmp(sentence, "parse model")) {
        } else {
            log_error(debug::category::main, "No task specified, exiting.");
            return;
        }
    }
    #if 0
    if(sentence) {
        /*  */ if(strview_cmp(sentence, "parse mesh")) {
            log_info(debug::category::main, "Mesh parsing");
            if(ctx.argc != 6) {
                log_error(debug::category::main,
                          "parse mesh [mesh file] [out directory]");
            } else {
                dirpath = ctx.argv[5];
                basedir = sys::dirname_view(ctx.argv[4]);

                            parse_mesh(ctx.argv[4], ctx.argv[5]);
            }
            return;
        } else if(strview_cmp(sentence, "parse cubemap")) {
    }
    #endif
}

