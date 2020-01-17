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


using namespace proto;

//u64 glsl_material_struct_def_string(StrBuffer buffer) {
//    u64 written = 0;
//    written += sprint(buffer.data, buffer.size, "layout(std140) uniform Material {\n");
//    for(auto& field : Material::Refl::fields) {
//        written += sprint(buffer.data + written, buffer.size - written,
//                          "    ", field.glsl_type, ' ', field.glsl_name, "; \n");
//    }
//    written += sprint(buffer.data + written, buffer.size - written, "};");
//    return written;
//}

u32 stream = 0;
u32 VAO = 0;
u32 bufsize = 1024 * 1024 * 50;

PROTO_SETUP {
    settings->shader_paths = "src/demos/sokoban/shaders/";
}

//ArrayMap<AssetHandle, Pair<Mesh, AssetMetadata>> map;
RenderBatch batch;
Entity cubes[3];

AssetHandle main_shader_h;
PROTO_INIT {
    auto& ctx = *context;
    ctx.exit_sig = true;


    StringView archive_path = "outmesh/sponza.pack";
    if(ser::Archive * archive = ser::open_archive(archive_path)) {
        for(u32 i=0; i<archive->nodes.size(); ++i) {
            if(archive->nodes[i].type == ser::Archive::Node::asset) {
                archive->loat_asset(i);
            }
        }
    } else
        log_error(debug::category::main, "Failed to open archive ", archive_path);

    batch.init(sizeof(Vertex) * 1024, sizeof(u32) * 1024);

    if( auto cube = cubes[0] = create_entity() ) {
        auto& render_mesh = add_component<RenderMeshComp>(cube);
        auto& transform = add_component<TransformComp>(cube);
        render_mesh.mesh_h = ctx.cube_h;
        render_mesh.color = vec3(1.0,0.0,0.0);
        transform.position = vec3(0.0,0.0,0.0);
    } else 
        debug_error(debug::category::main, "Failed to create entity");

    if( auto cube = cubes[1] = create_entity() ) {
        auto& render_mesh = add_component<RenderMeshComp>(cube);
        auto& transform = add_component<TransformComp>(cube);
        render_mesh.mesh_h = ctx.cube_h;
        render_mesh.color = vec3(0.0,1.0,0.0);
        transform.position = vec3(2.0,0.0,0.0);
    } else 
        debug_error(debug::category::main, "Failed to create entity");

    if( auto cube = cubes[2] = create_entity() ) {
        auto& render_mesh = add_component<RenderMeshComp>(cube);
        auto& transform = add_component<TransformComp>(cube);
        render_mesh.mesh_h = ctx.cube_h;
        render_mesh.color = vec3(0.0,0.0,1.0);
        transform.position = vec3(-2.0,0.0,0.0);
    } else 
        debug_error(debug::category::main, "Failed to create entity");


    main_shader_h = 
        create_asset_rref<ShaderProgram>("sokoban_main")
            .$_init()
            .$_attach_shader_file(ShaderType::Vert, "sokoban_main_vert.glsl")
            .$_attach_shader_file(ShaderType::Frag, "sokoban_main_frag.glsl")
            .$_link().handle;

    if(!main_shader_h)
        debug_warn(debug::category::main, "Could not find main shader.");

    glEnable(GL_DEPTH_TEST);
    ctx.camera.position = vec3(0.0, 0.0, 5.0);
    ctx.exit_sig = false;
}

PROTO_UPDATE {
    auto& ctx = *context;
    auto& time = ctx.clock.elapsed_time;

    for(auto& comp : ctx.comp.render_mesh) {
        if(!comp.flags.check(RenderMeshComp::batched_bit)) batch.add(comp);
    }

    for(auto cube : cubes) if(cube)
            get_component<TransformComp>(cube)->rotation = angle_axis(time, glm::normalize(vec3(cos(time), 1.0, sin(time))));

    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    get_asset_ref<ShaderProgram>(main_shader_h).use();

    batch.render();

    if(time > 3.0f)
        ctx.exit_sig = true;
}

