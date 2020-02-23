#include "optproc.hh"
#include "ar-common.hh"
#include "fetch.hh"

#include "proto/core/io.hh"
#include "proto/core/context.hh"
#include "proto/core/meta.hh"
#include "proto/core/platform/api.hh"
#include "proto/core/util/String.hh"
#include "proto/core/containers/StringArena.hh"
#include "proto/core/asset-system/interface.hh"

using namespace proto;

int opt_input_proc(Array<StringView>& args){
    if(args.count() == 0) {
        log_error(debug::category::main, "-input option requires one or more arguments.");
        return -1;
    }

    if(ar_flags.at(verbose_bit)) println("Loading files:");
    auto& ctx = *context;

    Array<String> filepaths; filepaths.init(&ctx.memory);
    defer { filepaths.dtor(); };

    for(auto path : args) {
        auto fp = sys::search_for_file(path, search_paths);

        if(!fp) {
            log_error(debug::category::main, "Could not find file ", path);
            return -1;
        } else
            filepaths.push_back(meta::move(fp));
    }

    for(auto& fp : filepaths) {
        if(!fetch(fp.view())) {
            log_error(debug::category::main, "Failed to fetch data from ", fp.view());
            return -1;
        }
    }

    if(ar_flags.at(verbose_bit)) println();
    return 0;
} 

int opt_output_proc(Array<StringView>& args) {
    if(args.count() != 1) {
        log_error(debug::category::main, "Option -output expects one argument.");
        return -1;
    }

    ser::Archive archive;
    auto outpath = args[0];

    if(ar_flags.at(verbose_bit)) println("Writing to archive: ", outpath);

    u64 ar_data_size_acc = 0;
    for(auto asset : loaded_assets)
        ar_data_size_acc += INVOKE_FTEMPL_WITH_ASSET_REF(ser::serialized_size, asset);

    ECSTreeMemLayout layout;
    if(auto ec = calc_ecs_tree_memlayout(layout, loaded_ents)) {
        log_error(debug::category::data, "Computing ECS Tree memory layout failed: ");
        return -1;
    }

    ar_data_size_acc += layout.size;

    if(auto ec = archive.create(outpath,
                                loaded_assets.size() + 2, // root + ecs_tree
                                ar_data_size_acc))
    {
        log_error(debug::category::main, ec.message());
        return -1;
    }

    defer {
        if(auto ec = archive.dtor())
            return (void)log_error(debug::category::main, ec.message());
    };

    
    for(auto asset : loaded_assets) {
        if(auto ec = archive.store(asset)) {
            log_error(debug::category::data,
                      "Archiving ", get_metadata(asset)->name, " failed: ", ec.message());
            break;
        }
    }


    if(auto ec = archive.store(loaded_ents, &layout)) {
        log_error(debug::category::data, "Archiving ECS Tree failed: ", ec.message());
    }

        //    for(auto& [mesh, metadata] : ctx.meshes.values) {
        //        // make them look like they are not in memory,
        //        // we want to load them from just written archive for test
        //        mesh.flags.unset(Mesh::cached_bit);
        //        metadata.archive_hash = archive.superblock->hash;
        //    }
        //
        //    u64 ar_size = archive.superblock->archive_size;
        //    const char * ar_size_unit;
        //    //
        //    u64 kb = 1024, mb = 1024 * 1024, gb = 1024 * 1024 * 1024;
        //
        //    if(ar_size < kb) {
        //        ar_size_unit = "bytes";
        //    } else if(ar_size < 1024 * 1024) {
        //        ar_size_unit = "kilobytes";
        //        ar_size /= kb;
        //    } else if(ar_size < gb) {
        //        ar_size_unit = "megabytes";
        //        ar_size /= mb;
        //    }
        //
        //    println_fmt("Parsed % meshes, % textures and % materials. Written to % (% %).",
        //                mesh_count, tex_count, mat_count, outpath, ar_size, ar_size_unit);
        //
        //
        //    #if 0
        //    #endif
        //
        //    return 0;

    if(ar_flags.at(verbose_bit)) println();
    return 0;
}

int opt_search_proc(Array<StringView>& args){
    for(auto path : args) {
        if(!sys::is_directory(path)) {
            log_error(debug::category::main, path, " is not a directory.");
            return -1;
        } else {
            search_paths.store(path); 
        }
    }
    return 0;
} 

int opt_verbose_proc(Array<StringView>& args){
    ar_flags.toggle(verbose_bit);
    return 0;
} 

int opt_preview_proc(Array<StringView>& args){
    ar_flags.toggle(preview_bit);
    return 0;
} 

int opt_list_proc(Array<StringView>& args){
    if(!args.count()) {
        log_error(debug::category::main, "Option -list expects one or more arguments.");
        return -1;
    }

    for(auto arg : args) {
        using Node = serialization::Archive::Node;
        ser::Archive archive;
        if(auto ec = archive.open(arg)) {
            log_error(debug::category::main, ec.message());
            return -1;

        } else {
            println(archive.superblock->name);
            for(auto& node : archive.nodes) {
                switch(node.type) {
                case Node::free:
                    print("Free Node "); break;
                case Node::directory:
                    print("Directory "); break;
                case Node::asset: 
                    print("Asset     "); break;
                case Node::ecs_tree: 
                    print("ECS Tree  "); break;
                default:
                    print("Unknown   ");
                }
                
                print(node.name);
                println();
            }

            if(auto ec = archive.dtor()) {
                log_error(debug::category::main, ec.message());
                return -1;
            }
        }
    }
    
    println();
    return 0;
} 



