#include "proto/proto.hh"
#include "proto/core/io.hh"
#include "proto/core/graphics/ShaderProgram.hh"
#include "proto/core/graphics/Mesh.hh"
#include "proto/core/platform/file_loaders.hh"
#include "proto/core/context.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/input.hh"
#include "proto/core/event-system.hh"
#include "proto/core/common/types.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/asset-system/serialization.hh"
#include "proto/core/math/hash.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/graphics/gl.hh"
#include "proto/core/util/namespace-shorthands.hh"
#include <stdio.h>
#include <glm/glm.hpp>
#include <tgmath.h>

#include <glm/gtc/matrix_transform.hpp>


float quad_vertices[] = {
    -0.5f,-0.5f, 0.0f, 0.0f, 0.0f,
     0.5f,-0.5f, 0.0f, 1.0f, 0.0f,
    -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
     0.5f, 0.5f, 0.0f, 1.0f, 1.0f
};

using namespace proto;

PROTO_SETUP {
    settings->asset_paths = "res/:res/external/crytek-sponza:res/gallery";
}
graphics::ShaderProgram
    main_shader,
    quad_shader,
    directional_light_shader,
    TBN_shader;
GLuint quad_VAO, quad_VBO;
GLuint dragon_VAO, dragon_VBO, dragon_EBO;
GLuint sponza_VAO, sponza_VBO, sponza_EBO;
GLuint test_VAO, test_VBO, test_EBO;
using Foo = void(*)(void);

proto::Context * ctx;

KeyInputSink key_input_sink;
MouseInputSink mouse_input_sink;

proto::vec3 dragon_pos(0.0,0.0,0.0);
proto::vec3 cam_pos(46.0,26.0,3.0);
proto::vec3 cam_rot(0.0,M_PI/2.0f,0.0);
proto::vec3 light_rot(0.0,0.0,0.0);

proto::vec3 sun_pos(4.0,100.0,-4.0);
proto::vec3 sun_dir = glm::normalize(vec3(-0.1,-1.0,-0.31));

float cam_speed = 30.0f;
float cam_rot_speed = 0.03f;
u32 shadow_map_FBO = 0;
AssetHandle shadow_map;

const char * quad_vert_src = R"glsl(
    #version 450 core
    layout(location = 0) in vec3 a_position;
    layout(location = 1) in vec2 a_uv;

    out VertOut {
        vec3 position;
        vec2 uv;
    } frag_in;

    void main() {
        frag_in.position = a_position;
        frag_in.uv = a_uv;
        gl_Position = vec4(a_position, 1.0);
    }
)glsl";

const char * quad_frag_src = R"glsl(
    #version 450 core
    in VertOut {
        vec3 position;
        vec2 uv;
    } frag_in;

    uniform sampler2D quad_tex;

    out vec4 frag_color;
    void main() {
        frag_color = texture(quad_tex, frag_in.uv);
    }
)glsl";

const char * main_vert_src = R"glsl(
    #version 450 core
    layout(location = 0) in vec3 a_position;
    layout(location = 1) in vec3 a_normal;
    layout(location = 2) in vec2 a_uv;

    out VertOut {
        vec3 position;
        vec3 normal;
        vec2 uv;
        vec4 dirlight_space_pos;
    } geom_in;

    uniform float u_time;
    uniform mat4 u_mvp;
    uniform mat4 u_model;
    uniform mat4 u_dirlight_matrix;
    uniform vec3 u_cam_position;

   void main() {
        geom_in.position = (u_model * vec4(a_position,1.0)).xyz;
        geom_in.normal = a_normal;
        geom_in.uv = a_uv;
        geom_in.dirlight_space_pos = 
            u_dirlight_matrix * (u_model * vec4(a_position, 1.0));
        gl_Position = u_mvp * vec4(a_position, 1.0);
    }
)glsl";

///////////////////////////////////////////////////////////////////////////
const char * main_geom_src = R"glsl(
#version 450 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec4 dirlight_space_pos;
} geom_in[];

out GeomOut {
    vec3 position;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 uv;
    vec4 dirlight_space_pos;
} frag_in;

uniform mat4 u_mvp;
uniform float u_time;

mat2x3 computeTB() {
    vec3 e1 = geom_in[1].position - geom_in[0].position; // edge between
    vec3 e2 = geom_in[2].position - geom_in[0].position;

    vec2 d1 = geom_in[1].uv - geom_in[0].uv; // uv deltas
    vec2 d2 = geom_in[2].uv - geom_in[0].uv;
    
    float inv_det = 1.0/(d1.x * d2.y - d1.y * d2.x);

    mat2x2 adj = {{-d2.y, d1.y}, { d2.x,-d1.x}};
    mat3x2 E = {{ e1.x, e2.x}, { e1.y, e2.y}, { e1.z, e2.z}};

    return transpose(inv_det * adj * E);
}

void emit(int index) {
    mat2x3 TB = computeTB();
    frag_in.position           = geom_in[index].position;
    frag_in.normal             = geom_in[index].normal;
    frag_in.tangent            = normalize(TB[0]);
    frag_in.bitangent          = normalize(TB[1]);
    frag_in.uv                 = geom_in[index].uv;
    frag_in.dirlight_space_pos = geom_in[index].dirlight_space_pos;
    gl_Position = gl_in[index].gl_Position; 
    EmitVertex();
}

void main() {
    int i; for(i=0; i<gl_in.length(); i++) {emit(i);}
    EndPrimitive();
}
)glsl";
///////////////////////////////////////////////////////////////////////////
const char * main_frag_src = R"glsl(
    #version 450 core

    in GeomOut {
        vec3 position;
        vec3 normal;
        vec3 tangent;
        vec3 bitangent;
        vec2 uv;
        vec4 dirlight_space_pos;
    } frag_in;

    uniform float u_time;
    uniform mat4 u_mvp;
    uniform mat4 u_model;
    uniform vec3 u_cam_pos;

    uniform struct Material {
        vec3 diffuse;
        vec3 ambient;
        vec3 specular;
        float shininess;
        sampler2D diffuse_map;
        sampler2D ambient_map;
        sampler2D specular_map;
        sampler2D bump_map;
    } u_material;

    uniform struct DirectionalLight {
        sampler2D shadow_map;
        vec3 direction;
    } u_dirlight[1];

    float dirlight_shadow() {
        vec3 normal = normalize(frag_in.normal);
        vec3 to_dirlight_dir = normalize(-u_dirlight[0].direction);
        vec3 proj_coords = 
            frag_in.dirlight_space_pos.xyz / frag_in.dirlight_space_pos.w;
        proj_coords = proj_coords * 0.5 + 0.5;
        float dirlight_depth = texture(u_dirlight[0].shadow_map, proj_coords.xy).r;
        float cam_depth = proj_coords.z;
        float bias = max(0.02 * (1.0 - dot(normal,to_dirlight_dir)), 0.0006);
        float shadow = (cam_depth-bias) > dirlight_depth ? 0.9 : 0.0;
        return shadow;
    }

    out vec4 frag_color;

void main() {


float global_ambient = 0.1;
vec3 light_color = vec3(1.0,1.0,1.0);

// PARALAX MAPPING

mat3 TBN = transpose(mat3(
    normalize(frag_in.tangent), 
    normalize(frag_in.bitangent), 
    normalize(frag_in.normal) ));

vec3 frag_pos = (u_model * vec4(frag_in.position, 1.0)).xyz;
vec3 tan_cam_dir = normalize(TBN * u_cam_pos - TBN * frag_pos);

float paralax = (sin(3.0 * u_time) + 1.0)/2.0 * 0.01;
float height = -texture(u_material.bump_map, frag_in.uv).r;    
vec2 p = tan_cam_dir.xy / tan_cam_dir.z * (height * paralax);
vec2 uv = frag_in.uv ;//- p;    

// PHONG

float angle_factor;
vec3 frag_ambient;
vec3 frag_specular;
vec3 frag_diffuse;

frag_ambient = 
    global_ambient * 
    u_material.ambient * 
    texture(u_material.ambient_map, uv).xyz;

vec3 normal = normalize(frag_in.normal);
vec3 to_dirlight_dir = normalize(-u_dirlight[0].direction);

angle_factor = clamp(dot(normal,to_dirlight_dir), 0.0, 1.0);

frag_diffuse = 
    angle_factor *
    u_material.diffuse *
    light_color *
    texture(u_material.diffuse_map, uv).xyz;

vec3 to_cam_dir = normalize(u_cam_pos - frag_in.position);

angle_factor = dot(reflect(-to_dirlight_dir, normal), to_cam_dir);
angle_factor = clamp(angle_factor, 0.0, 1.0);
angle_factor = pow(angle_factor, max(1.0,u_material.shininess));

float global_specular = 5.0f;
frag_specular =
    global_specular *
    angle_factor *
    light_color *
    u_material.specular *
    texture(u_material.specular_map, uv).xyz;

frag_diffuse *= (1.0 - angle_factor);

float shadow = clamp(1.0 -  dirlight_shadow(), 0.0, 1.0);
            
frag_color = vec4((frag_diffuse + frag_specular) * (shadow) + frag_ambient , 1.0);
//frag_color = texture(u_material.bump_map, uv);

}

)glsl";

/////////////////////////////////////////////////////////////////////

const char * TBN_vert_src = R"glsl(
#version 450 core
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;
 
out VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} geom_in;  

uniform mat4 u_mvp;
 
void main() { 
    geom_in.position = a_position;
    geom_in.normal = a_normal;
    geom_in.uv = a_uv;
    gl_Position = u_mvp * vec4(a_position,1.0); 
}

)glsl";

const char * TBN_geom_src = R"glsl(
#version 450 core
layout(triangles) in;
layout(line_strip, max_vertices = 6) out;

in VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} geom_in[];  

out GeomOut {
    vec3 position;
    vec3 color;
} frag_in;  

uniform mat4 u_mvp;

vec3 compute_normal() {
    vec3 e1 = (geom_in[0].position - geom_in[1].position);
    vec3 e2 = (geom_in[0].position - geom_in[2].position);
    return normalize(cross(e1, e2));
}

vec3 normal_color = vec3(0.0,0.0,1.0);
vec3 tangent_color = vec3(1.0,0.0,0.0);
vec3 bitangent_color = vec3(0.0,1.0,0.0);


mat2x3 computeTB() {
    vec3 e1 = geom_in[1].position - geom_in[0].position; // edge between
    vec3 e2 = geom_in[2].position - geom_in[0].position;

    vec2 d1 = geom_in[1].uv - geom_in[0].uv; // uv deltas
    vec2 d2 = geom_in[2].uv - geom_in[0].uv;
    
    float inv_det = 1.0/(d1.x * d2.y - d1.y * d2.x);

    mat2x2 adj = {{-d2.y, d1.y}, { d2.x,-d1.x}};
    mat3x2 E = {{ e1.x, e2.x}, { e1.y, e2.y}, { e1.z, e2.z}};

    return transpose(inv_det * adj * E);
}

void draw_vec(int index, vec3 dir, vec3 color) { 
    gl_Position = u_mvp * vec4(geom_in[index].position, 1.0); 
    frag_in.position = geom_in[index].position;
    frag_in.color = color;
    EmitVertex();

    vec3 tip = geom_in[index].position + dir * 0.1;
    gl_Position = u_mvp * vec4(tip, 1.0); 
    frag_in.position = tip;
    frag_in.color = color;
    EmitVertex();
    EndPrimitive();
}

void emit(int index) {
    mat2x3 TB = computeTB();
    vec3 tangent = normalize(TB[0]);
    vec3 bitangent = normalize(TB[1]);
    vec3 normal = geom_in[index].normal;

    draw_vec(index, normal, normal_color);
    draw_vec(index, tangent, tangent_color);
    draw_vec(index, bitangent, bitangent_color);
}

void main() {
    emit(0);
}
)glsl";

const char * TBN_frag_src = R"glsl(
#version 450 core
out vec4 frag_color;

in GeomOut {
    vec3 position;
    vec3 color;
} frag_in;  

uniform vec3 u_cam_pos;
uniform mat4 u_model;

void main() { 
    vec3 between = u_cam_pos - (u_model * vec4(frag_in.position, 1.0)).xyz;
    frag_color = vec4(frag_in.color ,pow(10.0/length(between),7.0) ); 
}
)glsl";

//const char * main_geom_src = R"glsl(
//    #version 450 core
//    layout(triangles) in;
//    
//    // Three lines will be generated: 6 vertices
//    layout(line_strip, max_vertices=6) out;
//    
//    uniform float normal_length;
//    uniform mat4 gxl3d_ModelViewProjectionMatrix;
//    
//    in Vertex {
//      vec4 normal;
//      vec4 color;
//    } vertex[];
//    
//    out vec4 vertex_color;
//    
//    void main()
//    {
//      int i;
//      for(i=0; i<gl_in.length(); i++)
//      {
//        vec3 P = gl_in[i].gl_Position.xyz;
//        vec3 N = vertex[i].normal.xyz;
//        
//        gl_Position = gxl3d_ModelViewProjectionMatrix * vec4(P, 1.0);
//        vertex_color = vertex[i].color;
//        EmitVertex();
//        
//        gl_Position = gxl3d_ModelViewProjectionMatrix * vec4(P + N * normal_length, 1.0);
//        vertex_color = vertex[i].color;
//        EmitVertex();
//        
//        EndPrimitive();
//      }
//    }
//)glsl";

const char * directional_light_vert_src = R"glsl(
    #version 450 core
    layout(location = 0) in vec3 a_position;

    uniform mat4 u_mv;

    void main() {
        gl_Position = u_mv * vec4(a_position, 1.0);
    }
)glsl";

const char * directional_light_frag_src = R"glsl(
    #version 450 core
    void main() {}
)glsl";

u32 mesh_index = 0;
void key_callback(KeyEvent& ev) {
    namespace kc = proto::input::keycode;
 
    if(ev.code == kc::escape) ctx->exit_sig = true;

    float inc = cam_speed * ctx->clock.delta_time;
    proto::vec3 cam_move_dir = proto::vec3();

    if(ev.code == kc::e) {
        mesh_index = (mesh_index + 1) % context->meshes.size();
        vardump(mesh_index);
    }

         if(ev.code == kc::w) cam_move_dir = proto::vec3( 0.0f, 0.0f,-1.0f);
    else if(ev.code == kc::s) cam_move_dir = proto::vec3( 0.0f, 0.0f, 1.0f);
    else if(ev.code == kc::a) cam_move_dir = proto::vec3( 1.0f, 0.0f, 0.0f);
    else if(ev.code == kc::d) cam_move_dir = proto::vec3(-1.0f, 0.0f, 0.0f);
    else if(ev.code == kc::r) cam_move_dir = proto::vec3( 0.0f,-1.0f, 0.0f);
    else if(ev.code == kc::f) cam_move_dir = proto::vec3( 0.0f, 1.0f, 0.0f);
         
    if(ev.code == kc::W) cam_move_dir = proto::vec3( 0.0f, 0.0f,-10.0f);
    proto::vec3 up = vec3(0.0f,1.0f,0.0f);

    proto::vec3 front;
    front = proto::vec3( cos(cam_rot.x) * sin(cam_rot.y),
                        -sin(cam_rot.x),
                         cos(cam_rot.x) * cos(cam_rot.y));
    
    proto::vec3 side = glm::cross(front, up);
    up = glm::cross(front, side);

    cam_pos += (cam_move_dir.x * side +
                cam_move_dir.y * up +
                cam_move_dir.z * front) * inc;

    //printf("(%f %f %f)\r", cam_pos.x , cam_pos.y, cam_pos.z);
    fflush(stdout);
}

void mouse_callback(MouseEvent& ev) {
    float factor = cam_rot_speed * ctx->clock.delta_time;
    cam_rot.y += ev.delta.x * factor;
    cam_rot.x += ev.delta.y * factor;
    //float limit = M_PI/2.0f;
    //if(cam_rot.x > limit) {
    //    cam_rot.x = limit;
    //} else if(cam_rot.x < -limit) {
    //    cam_rot.x = -limit;
    //}
}

//
//#define PROTO_ENUM_ENTRY(NAME, VALUE, INFO)           \
//    constexpr static ImplType NAME = VALUE;           \
//    constexpr static InfoType NAME##_info = INFO;     \
//
//#define PROTO_ENUM_BEGIN(NAME, IMPLTYPE, INFOTYPE)    \
//    struct NAME {                                     \
//        using ImplType = IMPLTYPE;                    \
//        using InfoType = INFOTYPE;                   
//
//#define PROTO_ENUM_END \
//    constexpr static element_info(ImplType element) { \
//    }                                                 \
//    };
//
//PROTO_ENUM_BEGIN(TestEnum, u32, struct { const char * name; })
//PROTO_ENUM_ENTRY(Entry1, 1, { "entry1" });
//PROTO_ENUM_ENTRY(Entry2, 2, { "entry2" });
//PROTO_ENUM_END

//void print_asset_tree(AssetMetadata& root_asset, u32 level = 0) {
//    for(u32 i=0; i<level; i++) io::print('\t');
//
//    io::println(root_asset.name);
//    for(u32 i=0; i<root_asset.deps.size(); i++)
//        print_asset_tree(root_asset.deps[i], level + 1)
//}

void print_asset_tree(AssetHandle handle, u32 level = 0) {
    assert(handle);
    AssetMetadata * metadata = get_metadata(handle);

    for(s32 i=0; i<level; i++) io::print('\t');

    io::print(metadata->name);
    //if(metadata->handle.type == AssetType::MaterialAsset) {
    //    printf(" %f",
    //           proto::context->
    //           assets.get_asset<Material>(metadata->handle)
    //           ->specular_color.x);
    //}
    io::println();
    if(metadata->deps.size() != 0){
        for(u32 i=0; i<metadata->deps.size(); i++) {
            print_asset_tree(metadata->deps[i], level + 1);
        }
    }
};

namespace sys = proto::platform;
namespace meta = proto::meta;
namespace ser = proto::serialization;

Texture tex;

Array<int> test() {
    Array<int> ret;
    ret.init_resize(10,&context->memory);
    vardump(ret.size());
    return ret;
}


PROTO_INIT {
    ctx = proto::context;

    key_input_sink.init(ctx->key_input_channel, key_callback);
    mouse_input_sink.init(ctx->mouse_input_channel, mouse_callback);

    //auto handle = parse_asset_file_rec("external/crytek-sponza/sponza.obj", ctx);
    //ser::save_asset_tree_rec(handle, "res/models/crytek-sponza");
    ser::load_asset_dir("res/models/crytek-sponza");
    //Array<int> ok;
    //ok.init_resize(10, &context->memory);

    for(int i=0; i<ctx->textures.size(); i++)
        gl::gpu_upload(&ctx->textures[i]);

    for(int i=0; i<ctx->meshes.size(); i++) {
        Mesh & mesh = *get_asset<Mesh>(ctx->meshes[i].handle);
        AssetMetadata & metadata = *get_metadata(ctx->meshes[i].handle);

        gl::gpu_upload(&ctx->meshes[i]);
    }

    assert(!glGetError());


    TBN_shader.init();
    TBN_shader.create_shader(graphics::ShaderType::Vert, TBN_vert_src);
    TBN_shader.create_shader(graphics::ShaderType::Geom, TBN_geom_src);
    TBN_shader.create_shader(graphics::ShaderType::Frag, TBN_frag_src);
    TBN_shader.link();

    main_shader.init();
    main_shader.create_shader(graphics::ShaderType::Vert, main_vert_src);
    main_shader.create_shader(graphics::ShaderType::Geom, main_geom_src);
    main_shader.create_shader(graphics::ShaderType::Frag, main_frag_src);
    main_shader.link();

    quad_shader.init();
    quad_shader.create_shader(graphics::ShaderType::Vert, quad_vert_src);
    quad_shader.create_shader(graphics::ShaderType::Frag, quad_frag_src);
    quad_shader.link();

    directional_light_shader.init();
    directional_light_shader.create_shader(graphics::ShaderType::Vert,
                                           directional_light_vert_src);
    directional_light_shader.create_shader(graphics::ShaderType::Frag,
                                           directional_light_frag_src);
    directional_light_shader.link();
    // --------------------

    glGenFramebuffers(1, &shadow_map_FBO);

    shadow_map = create_asset("shadow_map", "", AssetType<Texture>::index);

    gl::bind_texture(shadow_map);
    //gl::debug_print_texture_slots();

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_FBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                         get_asset<Texture>(shadow_map)->gl_id, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenBuffers     (1, &quad_VBO);
    glGenVertexArrays(1, &quad_VAO);
    glBindVertexArray(quad_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, quad_VBO);
 
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices),
                 quad_vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float),
                          (void*) (0) );

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float),
                          (void*) (sizeof(float) * 3) );
   //================================================================


}

int count = 0;

void render_mesh(Mesh * mesh, bool tmp = false) {
    assert(mesh);
    mesh->bind();
    for(u32 i=0; i<mesh->spans.size(); i++) {
        Material * material = &mesh->spans[i].material;

        if(material && tmp) {
            main_shader.set_uniform <GL_FLOAT_VEC3> ("u_material.ambient",
                                    &material->ambient_color);

            main_shader.set_uniform <GL_FLOAT_VEC3> ("u_material.diffuse",
                                    &material->diffuse_color);

            main_shader.set_uniform <GL_FLOAT_VEC3> ("u_material.specular",
                                    &material->specular_color);

            main_shader.set_uniform <GL_FLOAT> ("u_material.shininess",
                                    material->shininess);

            Texture * ambient_map = get_asset<Texture>(material->ambient_map);

            if(ambient_map)
                main_shader.set_uniform<GL_SAMPLER_2D>
                    ("u_material.ambient_map", gl::bind_texture(ambient_map));
            else
                main_shader.set_uniform<GL_SAMPLER_2D>
                    ("u_material.ambient_map",
                     gl::bind_texture(ctx->default_ambient_map));

            Texture * diffuse_map = get_asset<Texture>(material->diffuse_map);

            if(diffuse_map)
                main_shader.set_uniform<GL_SAMPLER_2D>
                    ("u_material.diffuse_map", gl::bind_texture(diffuse_map));
            else
                main_shader.set_uniform<GL_SAMPLER_2D>
                    ("u_material.diffuse_map",
                     gl::bind_texture(ctx->default_diffuse_map));

            Texture * specular_map = get_asset<Texture>(material->specular_map);

            if(specular_map)
                main_shader.set_uniform<GL_SAMPLER_2D>
                    ("u_material.specular_map", gl::bind_texture(specular_map));
            else
                main_shader.set_uniform<GL_SAMPLER_2D>
                    ("u_material.specular_map",
                     gl::bind_texture(ctx->default_specular_map));

            Texture * bump_map = get_asset<Texture>(material->bump_map);

            //if(bump_map)
            //    main_shader.set_uniform<GL_SAMPLER_2D>
            //        ("u_material.bump_map", gl::bind_texture(bump_map));
        }

        glDrawElements
            (GL_TRIANGLES, mesh->spans[i].index_count,
             GL_UNSIGNED_INT, (void*)(sizeof(u32) * mesh->spans[i].begin_index));
    }

        
}

PROTO_UPDATE {
    proto::mat4 identity(1.0f);

    proto::mat4 model = identity;
    //dragon_pos = proto::vec3(sin(ctx->clock.elapsed_time),0.0,0.0);
    dragon_pos = proto::vec3(0.0,0.0,0.0);
    model = glm::translate(model, dragon_pos);

    proto::mat4 view = identity;

    view = glm::rotate(view, -cam_rot.x, proto::vec3(1.0,0.0,0.0));
    view = glm::rotate(view, -cam_rot.y, proto::vec3(0.0,1.0,0.0));
    view = glm::rotate(view, -cam_rot.z, proto::vec3(0.0,0.0,1.0));
    view = glm::translate(view, -cam_pos);


    proto::mat4 projection =
        glm::perspective(45.0f,
                         (float)ctx->window_size.x / ctx->window_size.y,
                         1.1f, 3000.0f);
    proto::mat4 mvp = projection * view * model;

    count++;
    float dirlight_near_plane = 1.1f, dirlight_far_plane = 200.0f;
    float tmp = 50.0f;
    glm::mat4 dirlight_proj =
        glm::ortho(-tmp, tmp, -tmp, tmp,
                   dirlight_near_plane, dirlight_far_plane);  

    glm::mat4 dirlight_view =
        glm::lookAt(sun_pos, sun_pos + sun_dir, 
                    glm::vec3( 0.0f, 1.0f,  0.0f)); 

    glm::mat4 dirlight_mv_matrix = dirlight_proj * dirlight_view; 

    //proto::vec3 light_pos(4* sin(ctx->clock.elapsed_time/10.0f), 30.0f,
    //                      4* cos(ctx->clock.elapsed_time/10.0f));

    //sun_pos = light_pos;

    glViewport(0, 0, 4096, 4096);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_FBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    directional_light_shader.use();
    directional_light_shader.set_uniform
        <GL_FLOAT_MAT4> ("u_mv", &dirlight_mv_matrix);

    // shadowmap render
    render_mesh(&ctx->meshes[mesh_index]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, ctx->window_size.x, ctx->window_size.y);
    main_shader.use();

    main_shader.set_uniform<GL_FLOAT>
        ("u_time", proto::context->clock.elapsed_time);

    main_shader.set_uniform<GL_FLOAT_MAT4> ("u_mvp", &mvp);
    main_shader.set_uniform<GL_FLOAT_MAT4> ("u_model", &model);
    main_shader.set_uniform
        <GL_FLOAT_MAT4> ("u_dirlight_matrix", &dirlight_mv_matrix);
    main_shader.set_uniform<GL_FLOAT_VEC3> ("u_cam_pos", &cam_pos);
    main_shader.set_uniform<GL_FLOAT_VEC3> ("u_sun_pos", &sun_pos);

    main_shader.set_uniform <GL_FLOAT_VEC3> ("u_dirlight[0].direction", &sun_dir);
    main_shader.set_uniform <GL_SAMPLER_2D> ("u_dirlight[0].shadow_map",
                                             gl::bind_texture(shadow_map) );

   render_mesh(&ctx->meshes[mesh_index], true);

   TBN_shader.use();
   TBN_shader.set_uniform<GL_FLOAT_MAT4> ("u_model", &model);
   TBN_shader.set_uniform<GL_FLOAT_MAT4> ("u_mvp", &mvp);
   TBN_shader.set_uniform<GL_FLOAT_VEC3> ("u_cam_pos", &cam_pos);
   render_mesh(&ctx->meshes[mesh_index]);

   quad_shader.use();
   //quad_shader.set_uniform<GL_SAMPLER_2D>("quad_tex", gl::bind_texture(shadow_map));
   glBindVertexArray(quad_VAO);
   //glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}



