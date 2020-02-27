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

Err opt_input_proc(Array<StringView>& args){
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
    return SUCCESS;
} 

Err opt_output_proc(Array<StringView>& args) {
    if(args.count() != 1) {
        log_error(debug::category::main, "Option -output expects one argument.");
        return INVALID_ARG_ERR;
    }

    Archive archive;
    auto outpath = args[0];

    if(ar_flags.at(verbose_bit)) println("Writing to archive: ", outpath);

    u64 blk_sz = Archive::_def_block_size;

    u64 ar_data_size_acc = 0;
    for(auto asset : loaded_assets)
        ar_data_size_acc +=
            mem::align(INVOKE_FTEMPL_WITH_ASSET_REF(serialization::serialized_size, asset), blk_sz);

    //ECSTreeMemLayout layout;
    //if(auto ec = calc_ecs_tree_memlayout(layout, loaded_ents)) {
    //    log_error(debug::category::data, "Computing ECS Tree memory layout failed: ");
    //    return -1;
    //}

    //ar_data_size_acc += layout.size;

    if(auto ec = archive.create(outpath,
                                loaded_assets.size() + 2, // root + ecs_tree
                                2*ar_data_size_acc))
    {
        log_error(debug::category::main, errmsg(ec));
        return ec;
    }

    defer {
        if(auto ec = archive.dtor())
            return (void)log_error(debug::category::main, errmsg(ec));
    };

    for(auto asset : loaded_assets) {
        if(auto ec = archive.store(asset)) {
            log_error(debug::category::data,
                      "Archiving ", get_metadata(asset)->name, " failed: ", errmsg(ec));
            break;
        }
    }

    //if(auto ec = archive.store(loaded_ents, &layout)) {
    //    log_error(debug::category::data, "Archiving ECS Tree failed: ", errmsg(ec));
    //}

    if(ar_flags.at(verbose_bit)) println();
    return SUCCESS;
}

Err opt_search_proc(Array<StringView>& args){
    for(auto path : args) {
        if(!sys::is_directory(path)) {
            log_error(debug::category::main, path, " is not a directory.");
            return -1;
        } else {
            search_paths.store(path); 
        }
    }
    return SUCCESS;
} 

Err opt_verbose_proc(Array<StringView>& args){
    ar_flags.toggle(verbose_bit);
    return SUCCESS;
} 

Err opt_preview_proc(Array<StringView>& args){
    ar_flags.toggle(preview_bit);
    return SUCCESS;
} 

Err opt_list_proc(Array<StringView>& args){
    if(!args.count()) {
        log_error(debug::category::main, "Option -list expects one or more arguments.");
        return -1;
    }

    for(auto arg : args) {
        using Node = Archive::Node;
        Archive archive;
        if(auto ec = archive.open(arg)) {
            log_error(debug::category::main, errmsg(ec));
            return ec;

        } else {
            println(archive.sblk->name);
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
                log_error(debug::category::main, errmsg(ec));
                return -1;
            }
        }
    }
    
    println();
    return SUCCESS;
} 



