#include "proto/proto.hh"

#include "proto/core/reflection.hh" 
#include "proto/core/meta.hh" 
#include "proto/core/util/String.hh" 
#include "proto/core/util/Buffer.hh" 
#include "proto/core/graphics.hh" 
#include "proto/core/util/defer.hh" 
#include "proto/core/error-handling.hh" 
#include "proto/core/graphics/primitives.hh" 
#include "proto/core/entity-system.hh" 
#include "proto/core/util/Pair.hh" 
#include "proto/core/containers/ArrayMap.hh" 
#include "proto/core/util/namespace-shorthands.hh" 
#include "proto/core/serialization/Archive.hh" 
#include "proto/core/serialization/interface.hh" 

#include "proto/core/util/FunctionView.hh"
#include "proto/core/debug.hh"
#include "proto/core/format.hh"


using namespace proto;

PROTO_SETUP {
    settings->shader_paths = "src/demos/sokoban/shaders/";
}

RenderBatch batch;
Array<Entity> ents;

AssetHandle main_shader_h;

using namespace proto;


PROTO_INIT {

    auto& ctx = *context;
    ctx.exit_sig = true;
    ents.init(500, &ctx.memory);

    //ser::Archive
    StringView archive_path = "outmesh/sponza.pack";
    if(ser::Archive * archive = ser::open_archive(archive_path)) {

        for(u32 i=0; i<archive->nodes.size(); ++i) {

            if(archive->nodes[i].type == ser::Archive::Node::asset) {

                Entity e; AssetHandle h;
                /**/ if( !(h = archive->load_asset(i)) )
                    debug_error(debug::category::main, "Failed to load_asset ");   

                else if( !(e = create_entity()) )
                    debug_error(debug::category::main, "Failed to load ");   

                else {
                    if(h.type != AssetType<Mesh>::index) continue;

                    ents.push_back(e);
                    auto& render_mesh = add_comp<RenderMeshComp>(e);
                    auto& transform = add_comp<TransformComp>(e);
                    render_mesh.mesh_h = h;
                    render_mesh.color = vec3(1.0,0.0,0.0);

                    transform.position = vec3(0.0,-10.0,0.0);
                    transform.rotation = angle_axis(M_PI/2.0, vec3(0.0, 1.0, 0.0));
                    transform.scale = vec3(0.1);
                } 
            }
        }
    } else
        log_error(debug::category::main, "Failed to open archive ", archive_path);

    //batch.init(sizeof(Vertex) * 1024 * 1024 * 50, sizeof(u32) * 1024 * 1024 * 50);
    batch.init(sizeof(Vertex) * 5242880, sizeof(u32) * 5242880);

    for(int i=0; i<3; ++i) {
        if( auto cube = create_entity() ) {
            auto& render_mesh = add_comp<RenderMeshComp>(cube);
            auto& transform = add_comp<TransformComp>(cube);
            render_mesh.mesh_h = ctx.cube_h;
            render_mesh.color = vec3(1.0,0.0,0.0);
            transform.position = vec3(i * 2.0f - 2.0f,0.0,0.0);

            ents.push_back(cube);
        } else 
            debug_error(debug::category::main, "Failed to create entity");
    }

    main_shader_h = 
        create_asset_rref<ShaderProgram>("sokoban_main")
            .$_init()
            .$_attach_shader_file(ShaderType::Vert, "sokoban_main_vert.glsl")
            .$_attach_shader_file(ShaderType::Frag, "sokoban_main_frag.glsl")
            .$_link().handle;

    if(!main_shader_h)
        debug_warn(debug::category::main, "Could not find main shader.");

    glEnable(GL_DEPTH_TEST);
    ctx.camera.position = vec3(0.0, 0.0, 30.0);
    ctx.exit_sig = false;
    //vardump(ctx.comp.render_mesh.size());
}

PROTO_UPDATE {
    auto& ctx = *context;
    auto& time = ctx.clock.elapsed_time;

    for(auto& comp : get_comp_arr<RenderMeshComp>()) {
        if(!comp.flags.check(RenderMeshComp::batched_bit)) batch.add(comp);
    }

    //for(auto e : ents) if(e)
    //        get_component<TransformComp>(e)->rotation = angle_axis(time, glm::normalize(vec3(cos(time * 2.0), 1.0, sin(time))));

    ctx.camera.position = vec3(0.0, 0.0, 50.0 * (cos(time) * 0.5 + 0.5) + 5.0);
    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    get_asset_ref<ShaderProgram>(main_shader_h).use();

    //    vardump(batch.meshes.size());
                                      
    batch.render();

    if(time > 1.0f)
        ctx.exit_sig = true;
}

