#include "proto/proto.hh"
#include "proto/core/platform/common.hh" 
#include "proto/core/graphics.hh" 
#include "proto/core/util/namespace-shorthands.hh" 
#include "proto/core/asset-system/interface.hh" 
#include "proto/core/asset-system/serialization.hh" 
#include "proto/core/util/argparse.hh" 
#include "proto/core/format.hh" 
#include "proto/core/util/String.hh" 
#include "proto/core/serialization/Archive.hh" 
#include "proto/core/serialization/interface.hh" 

#include "assimp/Importer.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "fetch.hh"
#include "cli-common.hh"
// extern defs from cli-common.hh
proto::StringArena search_paths;
proto::StringArena loaded_texture_paths;
proto::Array<proto::AssetHandle> loaded_assets;
proto::Array<proto::MemBuffer> allocated_buffers;


using namespace proto;

PROTO_SETUP {
    //settings->mode = RuntimeSettings::terminal_mode_bit;
    settings->asset_paths = "res/";
    settings->shader_paths = "src/demos/sokoban/shaders/";
}
#define LEN(arr) (sizeof(arr)/sizeof(arr[0]))

#define FAIL(...) { \
    log_error(debug::category::main, __VA_ARGS__); return;}

StringView cmdline_sentences[] = {"parse assets"};

using Opt = argparse::Option;
Opt options[] = {
    {"output", 'o', Opt::required_bit | Opt::param_bit},
    {"search", 's', Opt::param_bit},
    {"preview", 'p', 0 },
    {"verbose", 'v', 0 },
};

ArrayMap<u64, StringView> matched_opt;

void display_help() {
    log_info(debug::category::main, "<HELPTEXT>");
}


void preview_init();



PROTO_INIT {

    
    assert(proto::context);
    auto& ctx = *proto::context;
    search_paths.init_split(".", ':', &ctx.memory);
    loaded_texture_paths.init(&ctx.memory);
    loaded_assets.init(&ctx.memory);
    allocated_buffers.init(&ctx.memory);
    matched_opt.init(&ctx.memory);

    defer{
        for(auto buf : allocated_buffers)
            ctx.memory.free(buf.data);
        loaded_texture_paths.dtor();
        allocated_buffers.dtor();
        search_paths.dtor();
        loaded_assets.dtor();
        matched_opt.dtor();
    };


    ctx.exit_sig = true;

    StringView sentence =
        argparse::match_sentence(cmdline_sentences, LEN(cmdline_sentences), 2);

    if(!sentence) { display_help(); return;}

    if(strview_cmp(sentence, "parse assets")) {

        StringView outpath;

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
            if( !strcmp(options[index].name, "output") ){ outpath = val; }
            if( !strcmp(options[index].name, "search") ){ search_paths.store(val); }
            if( !strcmp(options[index].name, "verbose") ){ cli_flags.set( cli_verbose_bit); }
            if( !strcmp(options[index].name, "preview") ){
                cli_flags.set( cli_preview_bit );
                log_warn(debug::category::main, "cli preview is not yet complete feature");
            }
        }

        for(auto path : search_paths)
            if(!sys::is_directory(path)) log_warn(debug::category::main, path, " is not a directory.");

        assert(outpath);

        Array<String> conf_filepaths; conf_filepaths.init_resize(filepaths.size(), &ctx.memory);
        defer { conf_filepaths.dtor(); };


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

        if(cli_flags.check(cli_verbose_bit)) {

            for(auto h : loaded_assets) {
                auto asset_type_info = AssetType(h);
                auto metadata = get_metadata(h);
                assert(metadata);

                print(asset_type_info.name, ' ', metadata->name);
                switch(asset_type_info.index) {
                case(AssetType<Mesh>::index): {
                    auto& mesh = get_asset_ref<Mesh>(h);
                    print("\t(", mesh.vertices_count, " vertices, ", mesh.indices_count, " indices)");
                } break;
                case(AssetType<Texture2D>::index): {
                    auto& texture = get_asset_ref<Texture2D>(h);
                    print("\t( size: ", texture.size, ", ", (u32)texture.channels, " channels)");
                } break;
                }

                println();
            }
            flush();
        }

        u32 mesh_count = 0, tex_count = 0, mat_count = 0;
        for(auto h : loaded_assets) {
            switch(AssetType(h).index) {
            case(AssetType<Mesh>::index): mesh_count++; break;
            case(AssetType<Texture2D>::index): tex_count++; break;
            case(AssetType<Material>::index): mat_count++; break;
            }
        }

        ser::Archive archive;

        u64 ar_data_size_acc = 0;
        for(auto asset : loaded_assets)
            ar_data_size_acc += INVOKE_FTEMPL_WITH_ASSET_REF(ser::serialized_size, asset);

        if(auto ec = archive.create(outpath, loaded_assets.size(), ar_data_size_acc))
            return (void)log_error(debug::category::main, ec.message());

        defer {
            if(auto ec = archive.dtor())
                return (void)log_error(debug::category::main, ec.message());
        };

        for(auto asset : loaded_assets) {
            if(auto ec = archive.store(asset)) {
                log_error(debug::category::data, "Archiving ", get_metadata(asset)->name, " failed. ", ec.message()); break;
            }
        }

        for(auto& [mesh, metadata] : ctx.meshes.values) {
            // make them look like they are not in memory, we want to load them from just written archive for test
            mesh.flags.unset(Mesh::cached_bit);
            metadata.archive_hash = archive.superblock->hash;
        }

        u64 ar_size = archive.superblock->archive_size;
        const char * ar_size_unit;
        //
        u64 kb = 1024, mb = 1024 * 1024, gb = 1024 * 1024 * 1024;

        if(ar_size < kb) {
            ar_size_unit = "bytes";
        } else if(ar_size < 1024 * 1024) {
            ar_size_unit = "kilobytes";
            ar_size /= kb;
        } else if(ar_size < gb) {
            ar_size_unit = "megabytes";
            ar_size /= mb;
        }

        println_fmt("Parsed % meshes, % textures and % materials. Written to % (% %).",
                    mesh_count, tex_count, mat_count, outpath, ar_size, ar_size_unit);

        if(cli_flags.check(cli_preview_bit)) {
            preview_init();
        } else {
            ctx.exit_sig = true;
        }

    } else {
        log_error(debug::category::main, "Tell me what to do");
        display_help();
    }
}

RenderBatch batch;
Array<Entity> prev_ents;
AssetHandle main_shader_h;

void preview_init() {
    auto& ctx = *context;
    ctx.exit_sig = false;

    prev_ents.init(&ctx.memory);
    batch.init(sizeof(Vertex) * 5242880, sizeof(u32) * 5242880);
        
    for(auto asset : loaded_assets) {
        switch(asset.type) {

        case(AssetType<Mesh>::index): {
            if( Entity model = prev_ents.push_back(create_entity()) ) {
                auto& render_mesh = *add_comp<RenderMeshComp>(model);
                auto& transform = *add_comp<TransformComp>(model);
                render_mesh.mesh_h = asset;

                auto& mesh = get_asset_ref<Mesh>(render_mesh.mesh_h);
                transform.position = vec3(0.0,0.0,0.0);
                transform.scale = vec3(1.0 / glm::length(mesh.bounds) );
            } else 
                debug_error(debug::category::main, "Failed to create entity");
        }

        }
    }

    main_shader_h = 
        create_asset_rref<ShaderProgram>("sokoban_main")
            .$_init()
            .$_attach_shader_file(ShaderType::Vert, "sokoban_main_vert.glsl")
            .$_attach_shader_file(ShaderType::Frag, "sokoban_main_frag.glsl")
            .$_link().handle;

    if(!main_shader_h)
        debug_warn(debug::category::main, "Could not find main shader.");

    if(ser::open_archive("outmesh/sponza.pack"));
    else println("FAILED"); 

    glEnable(GL_DEPTH_TEST);
    ctx.camera.position = vec3(0.0, 0.0, 5.2);
}

PROTO_UPDATE {
    //    auto& ctx = *context;
    //    auto& time = ctx.clock.elapsed_time;
    //
    //    for(auto& comp : ctx.comp.render_mesh) {
    //        if(!comp.flags.check(RenderMeshComp::batched_bit)) batch.add(comp);
    //    }
    //
    //    //for(auto ent : prev_ents) {
    //    //    //      get_component<TransformComp>(ent)->rotation = angle_axis(time, glm::normalize(vec3(cos(time), 1.0, sin(time))));
    //    //}
    //
    //    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);
    //    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //
    //    get_asset_ref<ShaderProgram>(main_shader_h).use();
    //
    //    batch.render();

    //if(time > 1.0f)
    //    ctx.exit_sig = true;
}


