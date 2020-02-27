#include "ar-common.hh"
#include "fetch.hh"
#include "optproc.hh"

#include "proto/proto.hh"
#include "proto/core/platform/common.hh" 
#include "proto/core/graphics.hh" 
#include "proto/core/util/namespace-shorthands.hh" 
#include "proto/core/asset-system/interface.hh" 
#include "proto/core/asset-system/serialization.hh" 
#include "proto/core/util/argparse.hh" 
#include "proto/core/util/String.hh" 
#include "proto/core/serialization/Archive.hh" 
#include "proto/core/serialization/interface.hh" 

#include "assimp/Importer.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"


using namespace proto;

// extern defs from cli-common.hh
StringArena search_paths;
StringArena loaded_texture_paths;
Array<AssetHandle> loaded_assets;
Array<Entity> loaded_ents;
Array<MemBuffer> allocated_buffers;
Bitset<2> ar_flags;

CmdOpt opts[] = {
    {"help",    "h" , "Prints this help."},
    {"input",   "i" , "One or more input file paths."},
    {"output",  "o" , "Outpus archive to specified file."},
    {"search",  "s" , "Input files search paths."},
    {"verbose", "v" , "Toggle verbose."},
    {"preview", "p" , "Toggle preview."},
    {"list",    "ls", "List contents of an archive."},
};

using OptProc = Err (*)(Array<StringView>&);

void print_help() {
    println("Options are processed in sequence,"
            "any option affects only options passed after it.");
    argparse_print_help(opts);
}

CmdOptArgMap optarg_map;
OptProc opt_proc_switch(StringView opt) {
    /**/ if( strview_cmp(opt, "input") )
        return opt_input_proc;

    else if( strview_cmp(opt, "output") )
        return opt_output_proc;

    else if( strview_cmp(opt, "search") )
        return opt_search_proc;

    else if( strview_cmp(opt, "verbose") )
        return opt_verbose_proc;

    else if( strview_cmp(opt, "preview") )
        return opt_preview_proc;

    else if( strview_cmp(opt, "list") )
        return opt_list_proc;

    log_error(debug::category::main, "Option ", opt," is unimplemented.");
    auto opt_unimpl_proc = [](Array<StringView>&) -> Err { return DBG_ERR; };
    return opt_unimpl_proc;
}

PROTO_SETUP {}

PROTO_INIT {
    assert(proto::context);
    auto& ctx = *proto::context;

    optarg_map.init(&ctx.memory);
    if(argparse(optarg_map, opts,'-')) argparse_print_help(opts);

    for(const auto& [opt, args] : optarg_map)
        if(strview_cmp(opt, "help"))
            return (void)print_help();

    search_paths.init(&ctx.memory);
    loaded_texture_paths.init(&ctx.memory);
    loaded_assets.init(&ctx.memory);
    loaded_ents.init(&ctx.memory);
    allocated_buffers.init(&ctx.memory);

    for(const auto& [opt, args] : optarg_map)
        if(opt_proc_switch(opt)(args)) break;

    for(auto buf : allocated_buffers)
        ctx.memory.free(buf.data);

    println(loaded_ents.count());

    search_paths.dtor();
    loaded_texture_paths.dtor();
    loaded_assets.dtor();
    loaded_ents.dtor();
    allocated_buffers.dtor();
}

#if 0
//RenderBatch batch;
//AssetHandle main_shader_h;
//
//void preview_init() {
//    auto& ctx = *context;
//    ctx.exit_sig = false;
//
//    prev_ents.init(&ctx.memory);
//    batch.init(sizeof(Vertex) * 5242880, sizeof(u32) * 5242880);
//        
//    for(auto asset : loaded_assets) {
//        switch(asset.type) {
//
//        case(AssetType<Mesh>::index): {
//            if( Entity model = prev_ents.push_back(create_entity()) ) {
//                auto& render_mesh = *add_comp<RenderMeshComp>(model);
//                auto& transform = *add_comp<TransformComp>(model);
//                render_mesh.mesh_h = asset;
//
//                auto& mesh = get_asset_ref<Mesh>(render_mesh.mesh_h);
//                transform.position = vec3(0.0,0.0,0.0);
//                transform.scale = vec3(1.0 / glm::length(mesh.bounds) );
//            } else 
//                debug_error(debug::category::main, "Failed to create entity");
//        }
//
//        }
//    }
//
//    main_shader_h = 
//        create_asset_rref<ShaderProgram>("sokoban_main")
//            .$_init()
//            .$_attach_shader_file(ShaderType::Vert, "sokoban_main_vert.glsl")
//            .$_attach_shader_file(ShaderType::Frag, "sokoban_main_frag.glsl")
//            .$_link().handle;
//
//    if(!main_shader_h)
//        debug_warn(debug::category::main, "Could not find main shader.");
//
//    if(ser::open_archive("outmesh/sponza.pack"));
//    else println("FAILED"); 
//
//    glEnable(GL_DEPTH_TEST);
//    ctx.camera.position = vec3(0.0, 0.0, 5.2);
//}

//PROTO_UPDATE {
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

#endif

