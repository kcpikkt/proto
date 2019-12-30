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

struct GLSLMaterialFieldRefl {
    const char * glsl_type = nullptr;
    const char * glsl_name = nullptr;
};


using namespace proto;

u64 glsl_material_struct_def_string(StrBuffer buffer) {
    u64 written = 0;
    written += sprint(buffer.data, buffer.size, "layout(std140) uniform Material {\n");
    for(auto& field : Material::Refl::fields) {
        written += sprint(buffer.data + written, buffer.size - written,
                          "    ", field.glsl_type, ' ', field.glsl_name, "; \n");
    }
    written += sprint(buffer.data + written, buffer.size - written, "};");
    return written;
}

struct StateErrExt : ErrCategoryCRTP<StateErrExt>{
    inline static ErrCode err4 = 0;
    inline static ErrCode err5 = 0;
    static ErrMessage message(ErrCode) {
        return "StateErrExt";
    } 
};

using Ext = StateErrExt;
struct StateErr : ErrCategoryCRTP<StateErr>, StateErrExt {
    inline static ErrCode err1 = 0;
    inline static ErrCode err2 = 0;
    static ErrMessage message(ErrCode code) {
        return Ext::message(code);
    } 
};

u32 stream = 0;
u32 VAO = 0;
u32 bufsize = 1024 * 1024 * 50;

PROTO_SETUP {
    settings->shader_paths = "src/demos/sokoban/shaders/";
}

AssetHandle main_shader_h;
PROTO_INIT {
    auto& ctx = *context;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &stream);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, stream);
    glBufferData(GL_ARRAY_BUFFER, bufsize, NULL, GL_STREAM_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                        (void*) (offsetof(struct Vertex, position)) );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                            (void*) (offsetof(struct Vertex, normal)) );

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                            (void*) (offsetof(struct Vertex, uv)) );


    if( auto cube = create_entity("cube1") ) {
        auto& render_mesh = add_component<RenderMeshComp>(cube);
    } else 
        debug_error(debug::category::main, "Failed to create entity");

    void * buffdata = glMapBufferRange(GL_ARRAY_BUFFER,0, sizeof(cube_vertices),
                                       GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_RANGE_BIT);
    if(buffdata) {
        memcpy(buffdata, cube_vertices, sizeof(cube_vertices));
        glUnmapBuffer(GL_ARRAY_BUFFER);
    } else {
        debug_error(debug::category::data, "map failed");
    }

    main_shader_h = 
        create_init_asset_rref<ShaderProgram>("sokoban_main")
            .$_attach_shader_file(ShaderType::Vert, "sokoban_main_vert.glsl")
            .$_attach_shader_file(ShaderType::Frag, "sokoban_main_frag.glsl")
            .$_link().handle;

    if(!main_shader_h)
        debug_warn(debug::category::main, "Could not find main shader.");

    glEnable(GL_DEPTH_TEST);
    ctx.camera.position = vec3(0.0, 0.0, 2.0);
}

PROTO_UPDATE {
    auto& ctx = *context;
    auto& time = ctx.clock.elapsed_time;

    mat4 mvp, model, view = ctx.camera.view(), projection = ctx.camera.projection();

    glViewport(0, 0, ctx.window_size.x, ctx.window_size.y);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
    model = mat4(1.0);
    model = translate(model, vec3(0.0)) * glm::toMat4(angle_axis(time, glm::normalize(vec3(cos(time),1.0,sin(time))) ));
    mvp = projection * view * model;

    auto& main_shader = get_asset_ref<ShaderProgram>(main_shader_h)
        .$_use()
        .$_set_mat4  ("u_mvp", &mvp);

    glDrawArrays (GL_TRIANGLES, 0, 36);
}
