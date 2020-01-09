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

#include "cli-common.hh"
#include "fetch.hh"
////#include "fetch_mesh.hh"
//#include "fetch_texture.hh"
//#include "fetch_cubemap.hh"
//#include "fetch_model_tree.hh"

using namespace proto;

PROTO_SETUP {
    settings->mode = RuntimeSettings::terminal_mode_bit;
    settings->asset_paths = "res/";
}
#define LEN(arr) (sizeof(arr)/sizeof(arr[0]))

#define FAIL(...) { \
    log_error(debug::category::main, __VA_ARGS__); return;}

StringView cmdline_sentences[] = {"parse assets"};

using Opt = argparse::Option;
Opt options[] = {
    {"output", 'o', Opt::required_bit | Opt::param_bit},
    {"search", 's', Opt::param_bit},
};

ArrayMap<u64, StringView> matched_opt;

void display_help() {
    log_info(debug::category::main, "<HELPTEXT>");
}

PROTO_INIT {
    assert(proto::context);
    auto& ctx = *proto::context;
    search_paths.init_split(".", ':', &ctx.memory);
    mem_buffers.init(&ctx.memory);

    matched_opt.init(&ctx.memory);

    StringView sentence =
        argparse::match_sentence(cmdline_sentences, LEN(cmdline_sentences), 2);

    if(!sentence) { display_help(); return;}

    if(strview_cmp(sentence, "parse assets")) {

        StringView output_dir;

        Array<StringView> filepaths; filepaths.init(&ctx.memory);

        if(argparse::match_options(options, LEN(options), matched_opt, filepaths)) {
            display_help(); return;
        }

        // cmdline stripped from options consists of filepaths and 4 words of command invocation
        if(filepaths.size() >= 4) for(u64 i=0; i<4; ++i) filepaths.erase(0);

        if(filepaths.size() == 0) {
            log_error(debug::category::main, "No asset filepaths supplied.");
            display_help();
            return;
        }

        for(auto [index, val] : matched_opt) {
            if( !strcmp(options[index].name, "output") ){ output_dir = val; }
            if( !strcmp(options[index].name, "search") ){ search_paths.store(val); }
        }

        for(auto path : search_paths)
            if(!sys::is_directory(path)) log_warn(debug::category::main, path, " is not a directory.");

        assert(output_dir);

        Array<String> conf_filepaths; conf_filepaths.init_resize(filepaths.size(), &ctx.memory);
        defer { conf_filepaths.destroy(); };


        for(u64 i=0; i<filepaths.size(); ++i) {
            conf_filepaths[i] = sys::search_for_file(filepaths[i], search_paths);

            if(!conf_filepaths[i]) {
                log_error(debug::category::main, "Could not find file ", filepaths[i]); return; }
        }

        for(auto& fp : conf_filepaths) {
            if(fetch(fp.view())) {
                log_error(debug::category::main, "Failed to fetch data from ", fp.view());
                return;
            }
        }

    } else {
        log_error(debug::category::main, "Tell me what to do");
        display_help();
    }
#if 0
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
#endif
    //#if 0
    //if(sentence) {
    //    /*  */ if(strview_cmp(sentence, "parse mesh")) {
    //        log_info(debug::category::main, "Mesh parsing");
    //        if(ctx.argc != 6) {
    //            log_error(debug::category::main,
    //                      "parse mesh [mesh file] [out directory]");
    //        } else {
    //            dirpath = ctx.argv[5];
    //            basedir = sys::dirname_view(ctx.argv[4]);

    //                        parse_mesh(ctx.argv[4], ctx.argv[5]);
    //        }
    //        return;
    //    } else if(strview_cmp(sentence, "parse cubemap")) {
    //}
    //#endif
}

