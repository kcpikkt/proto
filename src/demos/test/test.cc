#include "proto/proto.hh"
#include "proto/core/io.hh"
#include "proto/core/graphics/ShaderProgram.hh"
#include "proto/core/graphics/Mesh.hh"
#include "proto/core/graphics/primitives.hh"
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
#include "proto/core/graphics/rendering.hh"
#include "proto/core/util/namespace-shorthands.hh"
#include "proto/core/util/ModCounter.hh"
#include "proto/core/util/parsing.hh"
#include <stdio.h>
#include <glm/glm.hpp>
#include <tgmath.h>

#include <glm/gtc/matrix_transform.hpp>

#include "proto/core/util/String.hh"
#include <random>

#include "proto/core/math/geometry.hh"
#include "proto/core/math/common.hh"


float quad_vertices[] = {
    -1.0f,-1.0f, 0.0f, 0.0f, 0.0f,
     1.0f,-1.0f, 0.0f, 1.0f, 0.0f,
    -1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
     1.0f, 1.0f, 0.0f, 1.0f, 1.0f
};

using namespace proto;

PROTO_SETUP {
    settings->asset_paths = "res/:res/external/crytek-sponza:res/external/cube";
}
graphics::ShaderProgram
    main_shader,
    quad_shader,
    highlight_shader,
    depth_shader,
    skybox_shader,
    gizmo_shader,
    lamp_shader,
    gbuffer_shader,
    ssao_shader,
    deferred_shader,
    TBN_shader;

GLuint quad_VAO, quad_VBO;
GLuint dragon_VAO, dragon_VBO, dragon_EBO;
GLuint sponza_VAO, sponza_VBO, sponza_EBO;
GLuint test_VAO, test_VBO, test_EBO;
using Foo = void(*)(void);

proto::Context * ctx;

KeyInputSink key_input_sink;
MouseMoveInputSink mouse_move_input_sink;
MouseButtonInputSink mouse_button_input_sink;

proto::vec3 dragon_pos(0.0,0.0,0.0);
proto::vec3 cam_pos(46.0,26.0,3.0);
proto::vec3 cam_rot(0.0,M_PI/2.0f,0.0);
proto::vec3 light_rot(0.0,0.0,0.0);

proto::vec3 sun_pos(4.0,100.0,-4.0);
proto::vec3 sun_dir = glm::normalize(vec3(-0.4,-1.0,-0.12));
proto::vec3 lamp_pos = vec3(0.0,20.0,0.0);

float cam_speed = 30.0f;
float cam_rot_speed = 0.03f;
u32 shadow_map_FBO = 0;
u32 shadow_cubemap_FBO = 0;
u32 gbuf_FBO;
u32 gbuf_position_tex, gbuf_normal_tex, gbuf_albedo_spec_tex; 

u32 ssao_FBO;


u32 shadow_cubemap;
ivec2 shadow_cubemap_size;
AssetHandle shadow_map;

const char * quad_vert_src = R"glsl(
    #version 450 core
    layout(location = 0) in vec3 a_position;
    layout(location = 1) in vec2 a_uv;

    out VertOut {
        vec3 position;
        vec2 uv;
    } frag_in;

    uniform float u_time;
    uniform vec2 u_scale;
    uniform vec2 u_position;
    uniform vec2 u_resolution;

    void main() {
        frag_in.position = a_position * 0.5;
        frag_in.position += vec3(0.5, 0.5, 0.0);
        frag_in.position *= vec3((u_scale / u_resolution) * 2.0, 1.0);
        frag_in.position += vec3(-1.0, -1.0, 0.0);
        frag_in.position += vec3((u_position.xy / u_resolution) * 2.0, 0.0);

        frag_in.uv = a_uv;
        gl_Position = vec4(frag_in.position, 1.0);
    }
)glsl";

const char * quad_frag_src = R"glsl(
    #version 450 core
    in VertOut {
        vec3 position;
        vec2 uv;
    } frag_in;

    uniform float u_alpha = 1.0;
    uniform sampler2D quad_tex;

    out vec4 frag_color;
    void main() {
        frag_color = texture(quad_tex, frag_in.uv);
       // frag_color.w *= u_alpha;
    }
)glsl";


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

mat2x3 computeTB(int index) {
    int v0 = (index + 0)%3; 
    int v1 = (index + 1)%3; 
    int v2 = (index + 2)%3; 
    vec3 e1 = (geom_in[v1].position - geom_in[v0].position); // edge between
    vec3 e2 = (geom_in[v2].position - geom_in[v0].position);

    vec2 d1 = (geom_in[v1].uv - geom_in[v0].uv); // uv deltas
    vec2 d2 = (geom_in[v2].uv - geom_in[v0].uv);
    
    float inv_det = 1.0/(d1.x * d2.y - d1.y * d2.x);

    mat2x2 adj = {{d2.y, -d2.x}, { -d1.y,d1.x}};
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
    mat2x3 TB = computeTB(index);
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

const char * depth_vert_src = R"glsl(
    #version 450 core
    layout(location = 0) in vec3 a_position;

    uniform mat4 u_mv;

    void main() {
        gl_Position = u_mv * vec4(a_position, 1.0);
    }
)glsl";

const char * depth_frag_src = R"glsl(
    #version 450 core
    void main() {}
)glsl";

/////////////////////////////////////////////////////////////////

const char * skybox_vert_src = R"glsl(
#version 450
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

out vec3 tex_coords;
uniform mat4 u_mv;

void main() {
    tex_coords = a_position;
    gl_Position = u_mv * vec4(a_position, 1.0);
}
)glsl";

const char * skybox_frag_src = R"glsl(
#version 450
in vec3 tex_coords;
uniform samplerCube u_skybox;
out vec4 frag_color;
void main() {
    frag_color = texture(u_skybox, tex_coords);
}
)glsl";

const char * gizmo_vert_src = R"glsl(
#version 450
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

out VertOut {
    vec3 position;
    vec2 uv;
} frag_in;

uniform vec3 u_position;
uniform mat4 u_mvp;
uniform mat4 u_view;
uniform mat4 u_model;

void main() {
    frag_in.position = a_position;
    float gizmo_size = 3.0;
    frag_in.uv = (a_position.xy + 1.0)/2.0; //tmp, my uvs seem to be broken

    gl_Position = 
        u_mvp * vec4(u_position, 1.0) + 
        vec4(a_position * gizmo_size/2.0, 1.0);
}
)glsl";
const char * gizmo_frag_src = R"glsl(
#version 450

in VertOut {
    vec3 position;
    vec2 uv;
} frag_in;

uniform sampler2D u_icon;
out vec4 frag_color;

void main() {
    frag_color = texture(u_icon, frag_in.uv);
}
)glsl";
/////////////////////////////////////////////////////////////////
const char * lamp_vert_src = R"glsl(
#version 450 core
layout (location = 0) in vec3 a_position;
uniform mat4 u_model;
void main() {
    gl_Position = u_model * vec4(a_position,1.0);
})glsl";

const char * lamp_geom_src = R"glsl(
#version 450 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 u_shadow_t[6];
out vec4 frag_pos;

void main() {
    for(int face=0; face<6; face++) {
        gl_Layer = face;
        for(int i=0; i<3; i++) {
            frag_pos = gl_in[i].gl_Position;
            gl_Position = u_shadow_t[face] * frag_pos;
            EmitVertex();
        }
        EndPrimitive();
    }
})glsl";

const char * lamp_frag_src = R"glsl(
#version 450 core
in vec4 frag_pos;

uniform vec3 u_lamp_pos;
uniform float u_far_plane;

void main() {
    gl_FragDepth = length(frag_pos.xyz) / u_far_plane;
})glsl";


bool inspect_mode = true; 
u32 mesh_index = 0;
u32 span_index = 0;
float span_change_timestamp;

void key_callback(KeyEvent& ev) {
    namespace kc = proto::input::keycode;
 
    if(ev.code == kc::escape) ctx->exit_sig = true;

    float inc = cam_speed * ctx->clock.delta_time;
    proto::vec3 cam_move_dir = proto::vec3();

    if(ev.code == kc::v) {
        context->meshes[mesh_index].spans[span_index].flags.toggle
            (Mesh::Span::flip_uv_bit);
    }

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

void mouse_move_callback(MouseMoveEvent& ev) {
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

void mouse_button_callback(MouseButtonEvent& ev) {
    inspect_mode = !inspect_mode;
}

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

Texture2D tex;

Array<int> test() {
    Array<int> ret;
    ret.init_resize(10,&context->memory);
    return ret;
}

GLuint main_shader_ssbo;
struct _MainShaderSSBOStruct {
    u32 hash = invalid_asset_handle.hash;
    u32 span_index = 0;
    float depth = 100000000.0;
};

u32 skybox_cubemap_gl_id;
AssetHandle cube;
AssetHandle lightbulb;
AssetHandle ssao_map;

struct Light {
    vec3 position;
    vec3 color;
    AssetHandle shadow_map;
    float far;
    float intensity;
    int on;
};

Array<Light> pointlights;
Array<vec3> ssao_kernel; 


PROTO_INIT {
    ctx = proto::context;
    ctx->texture_slots[0].flags.set(OpenGLContext::TextureSlot::reserved_bit);
    ctx->texture_slots[1].flags.set(OpenGLContext::TextureSlot::reserved_bit);
    ctx->texture_slots[2].flags.set(OpenGLContext::TextureSlot::reserved_bit);
    ctx->texture_slots[3].flags.set(OpenGLContext::TextureSlot::reserved_bit);

    span_change_timestamp = context->clock.elapsed_time;
    //ctx->exit_sig = true;

    key_input_sink.init(ctx->key_input_channel,
                        key_callback);
    mouse_move_input_sink.init(ctx->mouse_move_input_channel,
                               mouse_move_callback);
    mouse_button_input_sink.init(ctx->mouse_button_input_channel,
                                 mouse_button_callback);

    ser::load_asset_dir("res/models/crytek-sponza");
    ///cube = create_asset("cube", "", AssetType<Mesh>::index);
    cube = parse_asset_file_rec("external/cube/cube.obj");
    Mesh * cube_mesh = get_asset<Mesh>(cube);
    //for(auto& v : cube_mesh->vertices) {
    //    printf("(%f, %f, %f)\n", v.position.x, v.position.y, v.position.z);
    //}

    //cube_mesh.
    //vardump(sizeof(cube_vertices) / sizeof(float));

    lightbulb  = parse_asset_file_rec("external/lightbulb.png");
    
    auto cubemap_rt = parse_asset_file_rec("external/sor_sea/sea_rt.JPG");
    auto cubemap_lf = parse_asset_file_rec("external/sor_sea/sea_lf.JPG");
    auto cubemap_up = parse_asset_file_rec("external/sor_sea/sea_up.JPG");
    auto cubemap_dn = parse_asset_file_rec("external/sor_sea/sea_dn.JPG");
    auto cubemap_ft = parse_asset_file_rec("external/sor_sea/sea_ft.JPG");
    auto cubemap_bk = parse_asset_file_rec("external/sor_sea/sea_bk.JPG");

    glGenTextures(1, &skybox_cubemap_gl_id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap_gl_id);

    Texture2D & cubemap_rt_texture = *get_asset<Texture2D>(cubemap_rt);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB,
                 cubemap_rt_texture.size.x, cubemap_rt_texture.size.y,
                 0, GL_RGB, GL_UNSIGNED_BYTE, cubemap_rt_texture.data);

    Texture2D & cubemap_lf_texture = *get_asset<Texture2D>(cubemap_lf);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB,
                 cubemap_lf_texture.size.x, cubemap_lf_texture.size.y,
                 0, GL_RGB, GL_UNSIGNED_BYTE, cubemap_lf_texture.data);

    Texture2D & cubemap_up_texture = *get_asset<Texture2D>(cubemap_up);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB,
                 cubemap_up_texture.size.x, cubemap_up_texture.size.y,
                 0, GL_RGB, GL_UNSIGNED_BYTE, cubemap_up_texture.data);

    Texture2D & cubemap_dn_texture = *get_asset<Texture2D>(cubemap_dn);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB,
                 cubemap_dn_texture.size.x, cubemap_dn_texture.size.y,
                 0, GL_RGB, GL_UNSIGNED_BYTE, cubemap_dn_texture.data);

    Texture2D & cubemap_ft_texture = *get_asset<Texture2D>(cubemap_ft);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB,
                 cubemap_ft_texture.size.x, cubemap_ft_texture.size.y,
                 0, GL_RGB, GL_UNSIGNED_BYTE, cubemap_ft_texture.data);

    Texture2D & cubemap_bk_texture = *get_asset<Texture2D>(cubemap_bk);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB,
                 cubemap_bk_texture.size.x, cubemap_bk_texture.size.y,
                 0, GL_RGB, GL_UNSIGNED_BYTE, cubemap_bk_texture.data);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);  


    ////////////////////////////////////////////////////////////////////////
    pointlights.init_resize(3, &context->memory);
    
    pointlights[0].shadow_map =
        create_asset("light_0_shadowmap", "", AssetType<Cubemap>::index);
    pointlights[1].shadow_map =
        create_asset("light_1_shadowmap", "", AssetType<Cubemap>::index);
    pointlights[2].shadow_map =
        create_asset("light_2_shadowmap", "", AssetType<Cubemap>::index);

    vec2 shadow_map_size = ivec2(1024);
    for(auto& light : pointlights) {
        get_asset<Cubemap>(light.shadow_map)->size = shadow_map_size;
        light.color = vec3(1.0f);
        light.far = 80.0f;
        light.intensity = 60.0f;
        light.on = 1;

        auto texunit = gl::bind_texture(get_asset<Cubemap>(light.shadow_map));
        assert(!glGetError());
        for(u8 i=0; i<6; i++)
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                         shadow_map_size.x, shadow_map_size.y, 0,
                         GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        gl::stale_texture_slot(texunit);
    }
        assert(!glGetError());

    glGenFramebuffers(1, &shadow_cubemap_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_cubemap_FBO);
    glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    assert(!glGetError());
    for(int i=0; i<ctx->textures.size(); i++) {
        gl::gpu_upload(&ctx->textures[i]);
        assert(!glGetError());
    }
    assert(!glGetError());

    for(int i=0; i<ctx->meshes.size(); i++) {
        Mesh & mesh = *get_asset<Mesh>(ctx->meshes[i].handle);
        AssetMetadata & metadata = *get_metadata(ctx->meshes[i].handle);
        io::println("uploaded ", metadata.name);
        gl::gpu_upload(&ctx->meshes[i]);
    }

    //log_info(1,gl::error_message());
    assert(!glGetError());

    highlight_shader.init();
    highlight_shader.create_shader_from_file(graphics::ShaderType::Vert,
                     "src/proto/shaders/highlight_vert.glsl");
    highlight_shader.create_shader_from_file(graphics::ShaderType::Frag,
                     "src/proto/shaders/highlight_frag.glsl");
    highlight_shader.link();

    gbuffer_shader.init();
    gbuffer_shader.create_shader_from_file(graphics::ShaderType::Vert,
                                           "src/proto/shaders/g-buffer_vert.glsl");
    gbuffer_shader.create_shader_from_file(graphics::ShaderType::Frag,
                                           "src/proto/shaders/g-buffer_frag.glsl");
    gbuffer_shader.link();

    deferred_shader.init();
    deferred_shader.create_shader_from_file(graphics::ShaderType::Vert,
                                            "src/proto/shaders/deferred_vert.glsl");
    deferred_shader.create_shader_from_file(graphics::ShaderType::Frag,
                                            "src/proto/shaders/deferred_frag.glsl");
    deferred_shader.link();

    ssao_shader.init();
    ssao_shader.create_shader_from_file(graphics::ShaderType::Vert,
                                        "src/proto/shaders/ssao_vert.glsl");
    ssao_shader.create_shader_from_file(graphics::ShaderType::Frag,
                                        "src/proto/shaders/ssao_frag.glsl");
    ssao_shader.link();

    TBN_shader.init();
    TBN_shader.create_shader(graphics::ShaderType::Vert, TBN_vert_src);
    TBN_shader.create_shader(graphics::ShaderType::Geom, TBN_geom_src);
    TBN_shader.create_shader(graphics::ShaderType::Frag, TBN_frag_src);
    TBN_shader.link();

    lamp_shader.init();
    lamp_shader.create_shader(graphics::ShaderType::Vert, lamp_vert_src);
    lamp_shader.create_shader(graphics::ShaderType::Geom, lamp_geom_src);
    lamp_shader.create_shader(graphics::ShaderType::Frag, lamp_frag_src);
    lamp_shader.link();

    main_shader.init();
    main_shader.create_shader_from_file(graphics::ShaderType::Vert,
                                        "src/proto/shaders/blinn-phong_vert.glsl");
    main_shader.create_shader_from_file(graphics::ShaderType::Geom,
                                        "src/proto/shaders/blinn-phong_geom.glsl");
    main_shader.create_shader_from_file(graphics::ShaderType::Frag,
                                        "src/proto/shaders/blinn-phong_frag.glsl");
    main_shader.link();

    quad_shader.init();
    quad_shader.create_shader(graphics::ShaderType::Vert, quad_vert_src);
    quad_shader.create_shader(graphics::ShaderType::Frag, quad_frag_src);
    quad_shader.link();

    skybox_shader.init();
    skybox_shader.create_shader(graphics::ShaderType::Vert, skybox_vert_src);
    skybox_shader.create_shader(graphics::ShaderType::Frag, skybox_frag_src);
    skybox_shader.link();

    gizmo_shader.init();
    gizmo_shader.create_shader(graphics::ShaderType::Vert, gizmo_vert_src);
    gizmo_shader.create_shader(graphics::ShaderType::Frag, gizmo_frag_src);
    gizmo_shader.link();

    depth_shader.init();
    depth_shader.create_shader(graphics::ShaderType::Vert, depth_vert_src);
    depth_shader.create_shader(graphics::ShaderType::Frag, depth_frag_src);
    depth_shader.link();

    // SHADOW MAP INIT ===============================================
    glGenFramebuffers(1, &shadow_map_FBO);

    shadow_map = create_asset("shadow_map", "", AssetType<Texture2D>::index);

    gl::bind_texture(shadow_map);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24,
                 4096, 4096, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  

    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_FBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                         get_asset<Texture2D>(shadow_map)->gl_id, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //================================================================

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
   // GBUFFER?
    glGenFramebuffers(1, &gbuf_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, gbuf_FBO);

    auto& scr_size = context->window_size;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &gbuf_position_tex);
    glBindTexture(GL_TEXTURE_2D, gbuf_position_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F,
                 scr_size.x, scr_size.y, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, gbuf_position_tex, 0);

    glGenTextures(1, &gbuf_normal_tex);
    glBindTexture(GL_TEXTURE_2D, gbuf_normal_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F,
                 scr_size.x, scr_size.y, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
                           GL_TEXTURE_2D, gbuf_normal_tex, 0);

    glGenTextures(1, &gbuf_albedo_spec_tex);
    glBindTexture(GL_TEXTURE_2D, gbuf_albedo_spec_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                 scr_size.x, scr_size.y, 0, GL_RGB, GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2,
                           GL_TEXTURE_2D, gbuf_albedo_spec_tex, 0);

    u32 attachments[3] = {GL_COLOR_ATTACHMENT0,
                          GL_COLOR_ATTACHMENT1,
                          GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);

    u32 gbuf_depth;
    glGenRenderbuffers(1, &gbuf_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, gbuf_depth);
    glRenderbufferStorage
        (GL_RENDERBUFFER, GL_DEPTH_COMPONENT, scr_size.x, scr_size.y);
    glFramebufferRenderbuffer
        (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gbuf_depth);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        debug_warn(debug::category::graphics,
                   "Incomplete framebuffer");
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //================================================================
    // SSAO ===============================================
    glGenFramebuffers(1, &ssao_FBO);

    ssao_map = create_asset("ssao_map", "", AssetType<Texture2D>::index);
    gl::bind_texture(ssao_map);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
                 ctx->window_size.x, ctx->window_size.y, 0, GL_RGB, GL_FLOAT, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, ssao_FBO);
    // BUG
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         get_asset<Texture2D>(ssao_map)->gl_id, 0);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer(GL_NONE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    ssao_kernel.init_resize(16, &context->memory);
    // SSAO KERNEL
    std::uniform_real_distribution<float> rand_floats(0.0,1.0);
    std::default_random_engine gen;
    for(u32 i=0; i<16; i++) {
        vec3 sample(rand_floats(gen) * 2.0 - 1.0,
                    rand_floats(gen) * 2.0 - 1.0,
                    rand_floats(gen));
        sample = glm::normalize(sample);
        sample *= rand_floats(gen);
        float scale = (float)i / 16.0f;
        sample *= lerp(0.1f, 1.0f, scale * scale);
        ssao_kernel[i] = sample;
    }
    //================================================================

    glGenBuffers(1, &main_shader_ssbo);
    _MainShaderSSBOStruct main_shader_ssbo_data;
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, main_shader_ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(main_shader_ssbo_data),
                 (void*)&main_shader_ssbo_data, GL_DYNAMIC_READ);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, main_shader_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

///////////////////////////////////////////////////////////////////////////////
int count = 0;
PROTO_UPDATE {

    glActiveTexture(GL_TEXTURE1);
    glBindTexture
        (GL_TEXTURE_2D,
         get_asset<Texture2D>(ctx->meshes[0].spans[377]
                              .material.diffuse_map)->gl_id);
    proto::mat4 identity(1.0f);

    proto::mat4 model = identity;
    //dragon_pos = proto::vec3(sin(ctx->clock.elapsed_time),0.0,0.0);
    dragon_pos = proto::vec3(0.0,0.0,0.0);
    model = glm::translate(model, dragon_pos);
    //model = glm::translate(model, vec3());

    proto::mat4 view = identity;

    view = glm::rotate(view, -cam_rot.x, proto::vec3(1.0,0.0,0.0));
    view = glm::rotate(view, -cam_rot.y, proto::vec3(0.0,1.0,0.0));
    view = glm::rotate(view, -cam_rot.z, proto::vec3(0.0,0.0,1.0));
    view = glm::translate(view, -cam_pos);

    proto::mat4 projection =
        perspective(45.0f,
                         (float)ctx->window_size.x / ctx->window_size.y,
                         1.1f, 3000.0f);
    proto::mat4 mvp = projection * view * model;

    float dirlight_near_plane = 1.1f, dirlight_far_plane = 200.0f;
    float tmp = 80.0f;
    glm::mat4 dirlight_proj =
        glm::ortho(-tmp, tmp, -tmp, tmp,
                   dirlight_near_plane, dirlight_far_plane);  

    sun_pos = -sun_dir * 100.0f;
    glm::mat4 dirlight_view =
        glm::lookAt(sun_pos, sun_pos + sun_dir, 
                    glm::vec3( 0.0f, 1.0f,  0.0f)); 

    glm::mat4 dirlight_mv_matrix = dirlight_proj * dirlight_view; 

    glViewport(0, 0, 4096, 4096);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_map_FBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    depth_shader.use();

    depth_shader.set_uniform <GL_FLOAT_MAT4> ("u_mv", &dirlight_mv_matrix);
    gfx::render_mesh(&ctx->meshes[mesh_index], true);

    float time = 1.2f * context->clock.elapsed_time;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // POINT LIGHTS MAPS ///////////////////////////////////////////////////////
    lamp_shader.use();

    pointlights[0].position =
        vec3(cos(time), sin(time), pow(sin(time/2.0f), 3)) * 4.0f + 
        vec3(cos(time/3.0f)*23.0f, 12.0f, 0.0) ;

    pointlights[1].position =
        vec3(-cos(time), sin(time/3.0f), pow(sin(time/2.0f + M_PI/2), 3)) * 4.0f + 
        vec3(cos(time + 1.0f), 12.0f, 0.0) ;

    pointlights[2].position =
        vec3(sin(time/2.0f), -cos(time), pow(sin(time/4.0f), 3)) * 2.0f + 
        vec3(0.0, (sin(time/5.0f) + 2.0f) * 10.0f, 0.0) ;


    pointlights[0].color = normalize(vec3(0.9,0.5,0.7));
    pointlights[1].color = normalize(vec3(0.8,0.7,0.4));
    pointlights[2].color = normalize(vec3(0.8,0.9,0.6));

    pointlights[0].on = 1; pointlights[1].on = 1; pointlights[2].on = 1;

    lamp_pos = vec3(0.0);
    float aspect = (float)shadow_cubemap_size.x / (float)shadow_cubemap_size.y;
    float near = 3.0f, far = 80.0f;
    Array<mat4> shadow_transforms; shadow_transforms.init(6, &ctx->memory);

    mat4 shadow_proj = perspective(M_PI/2, 1.0, near, far);

    shadow_transforms.push_back
        (glm::lookAt(vec3(0.0), vec3( 1.0,0.0,0.0), vec3(0.0,-1.0,0.0)));
    shadow_transforms.push_back
        (glm::lookAt(vec3(0.0), vec3(-1.0,0.0,0.0), vec3(0.0,-1.0,0.0)));
    shadow_transforms.push_back
        (glm::lookAt(vec3(0.0), vec3(0.0, 1.0,0.0), vec3(0.0, 0.0,1.0)));
    shadow_transforms.push_back
        (glm::lookAt(vec3(0.0), vec3(0.0,-1.0,0.0), vec3(0.0, 0.0,-1.0)));
    shadow_transforms.push_back
        (glm::lookAt(vec3(0.0), vec3(0.0,0.0, 1.0), vec3(0.0,-1.0,0.0)));
    shadow_transforms.push_back
        (glm::lookAt(vec3(0.0), vec3(0.0,0.0,-1.0), vec3(0.0,-1.0,0.0)));
 
    for(auto& t : shadow_transforms) t = shadow_proj * t;

    lamp_shader.set_uniform<GL_FLOAT>("u_far_plane", far);
    lamp_shader.set_uniform<GL_FLOAT_MAT4>("u_model", &model);
    lamp_shader.set_uniform<GL_FLOAT_MAT4>("u_shadow_t[0]", &shadow_transforms[0]);
    lamp_shader.set_uniform<GL_FLOAT_MAT4>("u_shadow_t[1]", &shadow_transforms[1]);
    lamp_shader.set_uniform<GL_FLOAT_MAT4>("u_shadow_t[2]", &shadow_transforms[2]);
    lamp_shader.set_uniform<GL_FLOAT_MAT4>("u_shadow_t[3]", &shadow_transforms[3]);
    lamp_shader.set_uniform<GL_FLOAT_MAT4>("u_shadow_t[4]", &shadow_transforms[4]);
    lamp_shader.set_uniform<GL_FLOAT_MAT4>("u_shadow_t[5]", &shadow_transforms[5]);
 
    glBindFramebuffer(GL_FRAMEBUFFER, shadow_cubemap_FBO);

    mat4 shadowmap_model;
    for(auto& light : pointlights) {
        Cubemap& shadow_map = *get_asset<Cubemap>(light.shadow_map);
        glViewport(0,0, shadow_map.size.x, shadow_map.size.y);

        glFramebufferTexture(GL_FRAMEBUFFER,
                             GL_DEPTH_ATTACHMENT, shadow_map.gl_id, 0);
        glClear(GL_DEPTH_BUFFER_BIT);
        auto texslot = gl::bind_texture(shadow_map);

        shadowmap_model = identity;
        shadowmap_model = glm::translate(shadowmap_model, -light.position);

        lamp_shader.set_uniform<GL_FLOAT_MAT4>("u_model", &shadowmap_model);

        gfx::render_mesh(&ctx->meshes[mesh_index], true);

        gl::stale_texture_slot(texslot);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

   // SKYBOX PRE-RENDERING ===================================================

    glClearColor(1.0f,1.0f,1.0f,1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, ctx->window_size.x, ctx->window_size.y);

    glDepthMask(GL_FALSE);
    glCullFace(GL_FRONT);
    auto & cube_mesh = (*get_asset<Mesh>(cube));

    skybox_shader.use();
    mat4 skybox_mv = projection * mat4(mat3(view));
    skybox_shader.set_uniform<GL_FLOAT_MAT4> ("u_mv", &skybox_mv);
    auto begin_index = cube_mesh.spans[0].begin_index;
    auto index_count = cube_mesh.spans[0].index_count;
    cube_mesh.bind();

    //skybox_shader.set_uniform<GL_SAMPLER_2D>("u_skybox",
    //        gl::bind_texture(pointlights[0].shadow_map));


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_cubemap_gl_id);
    glDrawArrays(GL_TRIANGLES, 0, cube_mesh.vertices.size());

    glDepthMask(GL_TRUE);
    glCullFace(GL_BACK);

    vec2 resolution;
    // MAIN RENDERING ===================================================
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, main_shader_ssbo);

    _MainShaderSSBOStruct main_shader_ssbo_data;
    main_shader_ssbo_data.hash = context->meshes[mesh_index].handle.hash;
    main_shader_ssbo_data.span_index = span_index;
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(main_shader_ssbo_data),
                 (void*)&main_shader_ssbo_data, GL_DYNAMIC_READ);

    glDisable(GL_BLEND);
    // GBUFFER

    glBindFramebuffer(GL_FRAMEBUFFER, gbuf_FBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    gbuffer_shader.use();

    gbuffer_shader.set_uniform<GL_FLOAT> ("u_time", ctx->clock.elapsed_time);
    gbuffer_shader.set_uniform<GL_FLOAT_MAT4> ("u_mvp", &mvp);
    gbuffer_shader.set_uniform<GL_FLOAT_MAT4> ("u_model", &model);
    gbuffer_shader.set_uniform<GL_FLOAT_MAT4> ("u_view", &view);
    gbuffer_shader.set_uniform<GL_FLOAT_MAT4> ("u_projection", &projection);

    gfx::render_mesh(&ctx->meshes[mesh_index]);
    //gfx::render_mesh(get_asset<Mesh>(cube));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // SSAO
    ssao_shader.use();
    #if 0
    // DEFERRED RENDER
    deferred_shader.use();

    // FIXME
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuf_position_tex) ;
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gbuf_normal_tex);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gbuf_albedo_spec_tex);

    deferred_shader.set_uniform<GL_SAMPLER_2D> ("gbuf.position"   , 1);
    deferred_shader.set_uniform<GL_SAMPLER_2D> ("gbuf.normal"     , 2);
    deferred_shader.set_uniform<GL_SAMPLER_2D> ("gbuf.albedo_spec", 3);

    deferred_shader.set_uniform<GL_FLOAT> ("u_time", ctx->clock.elapsed_time);
    deferred_shader.set_uniform<GL_FLOAT_MAT4> ("u_mvp", &mvp);
    deferred_shader.set_uniform<GL_FLOAT_VEC3> ("u_cam_pos", &cam_pos);
    deferred_shader.set_uniform <GL_FLOAT_MAT4> ("u_dirlight[0].matrix",
                                                 &dirlight_mv_matrix);
    deferred_shader.set_uniform<GL_FLOAT_VEC3> ("u_dirlight[0].direction", &sun_dir);
    vec3 white = vec3(0.8,0.7,0.6);
    deferred_shader.set_uniform<GL_FLOAT_VEC3> ("u_dirlight[0].color", &white);
    deferred_shader.set_uniform<GL_FLOAT> ("u_dirlight[0].intensity", 0.8f);
    deferred_shader.set_uniform<GL_SAMPLER_2D> ("u_dirlight[0].shadow_map",
                                                gl::bind_texture(shadow_map));

    for(u32 i=0; i<pointlights.size(); i++) {
        auto& light = pointlights[i];

        char uniform_name[32];
        sprint(uniform_name, 24, "u_pointlight[", i, "].");
        char * sub_uniform_name = uniform_name + 16;

        sprint(sub_uniform_name, 15, "position\0");
        deferred_shader.set_uniform<GL_FLOAT_VEC3> (uniform_name, &light.position);
        sprint(sub_uniform_name, 15, "color\0");
        deferred_shader.set_uniform<GL_FLOAT_VEC3> (uniform_name, &light.color);
        sprint(sub_uniform_name, 15, "far\0");
        deferred_shader.set_uniform<GL_FLOAT> (uniform_name, light.far);
        sprint(sub_uniform_name, 15, "intensity\0");
        deferred_shader.set_uniform<GL_FLOAT> (uniform_name, light.intensity);
        sprint(sub_uniform_name, 15, "on\0");
        deferred_shader.set_uniform<GL_INT> (uniform_name, light.on);

        sprint(sub_uniform_name, 15, "shadow_map\0");
        deferred_shader.set_uniform<GL_SAMPLER_2D>
            (uniform_name, gl::bind_texture(get_asset<Cubemap>(light.shadow_map)));

    }
    glBindVertexArray(quad_VAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    #endif

    ////////////////////////////////////////////////////////////////////
 
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, main_shader_ssbo);

    auto * center_mesh = (_MainShaderSSBOStruct *)
        glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

    if(span_index != center_mesh->span_index) {
        span_change_timestamp = context->clock.elapsed_time;
        span_index = center_mesh->span_index;
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

    #if 0
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    gizmo_shader.use();
    glBindVertexArray(quad_VAO);

    mat4 gizmo_model = identity;
    //gizmo_model = glm::rotate(gizmo_model, (float)(M_PI/2.0), vec3(0.0,1.0,0.0));
    //gizmo_model = glm::translate(gizmo_model, lamp_pos);
    mat4 gizmo_mvp = projection * view * gizmo_model;

    gizmo_shader.set_uniform<GL_FLOAT_MAT4> ("u_view", &view);
    gizmo_shader.set_uniform<GL_FLOAT_MAT4> ("u_model", &gizmo_model);
    gizmo_shader.set_uniform<GL_FLOAT_MAT4> ("u_mvp", &gizmo_mvp);
    gizmo_shader.set_uniform<GL_SAMPLER_2D> ("u_icon", gl::bind_texture(lightbulb));

    for(u32 i=0; i<pointlights.size(); i++) {
        auto& light = pointlights[i]; if(!light.on) continue;
        gizmo_shader.set_uniform<GL_FLOAT_VEC3> ("u_position", &light.position);

        glBindVertexArray(quad_VAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
    #endif

    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if(inspect_mode || true) {
        glDisable(GL_DEPTH_TEST);

        #if 0
        highlight_shader.use();
        highlight_shader.set_uniform<GL_FLOAT_MAT4> ("u_mvp", &mvp);

        vec4 highlight_color = vec4(1.0, 1.0, 1.0,
                 min(0.1, ctx->clock.elapsed_time - span_change_timestamp));

        highlight_shader.set_uniform<GL_FLOAT_VEC4> ("u_highlight_color",
                                                     &highlight_color);

        ctx->meshes[mesh_index].bind();
        gfx::render_span(&ctx->meshes[mesh_index], span_index, true);
        #endif

        vec2 scale, position;
        quad_shader.use();
        glBindVertexArray(quad_VAO);

        scale = vec2(58.0 + 64.0 + 16.0 ,24.0 * 32 + 64.0);
        position = vec2(0.0, resolution.y - scale.y);
        quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_resolution", &resolution);
        quad_shader.set_uniform<GL_SAMPLER_2D>
            ("quad_tex", gl::bind_texture(context->default_bump_map));

        quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_scale", &scale);
        quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_position", &position);
        quad_shader.set_uniform<GL_FLOAT> ("u_alpha", 0.3f);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        quad_shader.set_uniform<GL_FLOAT> ("u_alpha", 1.0f);
        Material& material = ctx->meshes[mesh_index].spans[span_index].material;

        scale = vec2(32.0);
        position = vec2(16.0,resolution.y - 32.0);
        quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_scale", &scale);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, gbuf_position_tex) ;
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, gbuf_normal_tex);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, gbuf_albedo_spec_tex);

        //gl::debug_print_texture_slots();
        s32 index = 0;
        for(auto& slot : context->texture_slots) {
            position.y -= scale.y;
            using Slot = OpenGLContext::TextureSlot;
            quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_position", &position);
            quad_shader.set_uniform<GL_SAMPLER_2D> ("quad_tex", index);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            index++;
        }

        scale = vec2(64.0,64.0);
        position = vec2(58.0,resolution.y);
        quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_scale", &scale);
        quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_position", &position);

        position.y -= scale.y + 32.0;
        if(material.ambient_map) {
            quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_position", &position);
            quad_shader.set_uniform<GL_SAMPLER_2D>
                ("quad_tex", gl::bind_texture(material.ambient_map));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        position.y -= scale.y + 32.0;
        if(material.diffuse_map){
            quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_position", &position);
            quad_shader.set_uniform<GL_SAMPLER_2D>
                ("quad_tex", gl::bind_texture(material.diffuse_map));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        position.y -= scale.y + 32.0;
        if(material.specular_map){
            quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_position", &position);
            quad_shader.set_uniform<GL_SAMPLER_2D>
                ("quad_tex", gl::bind_texture(material.specular_map));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        position.y -= scale.y + 32.0;
        if(material.bump_map){
            quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_position", &position);
            quad_shader.set_uniform<GL_SAMPLER_2D>
                ("quad_tex", gl::bind_texture(material.bump_map));
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }

        quad_shader.use();
        glBindVertexArray(quad_VAO);
        scale = vec2(256.0,256.0);
        position = vec2(32.0,32.0);
        quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_resolution", &resolution);
        quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_scale", &scale);
        quad_shader.set_uniform<GL_FLOAT_VEC2> ("u_position", &position);
        quad_shader.set_uniform<GL_SAMPLER_2D> ("quad_tex",
                                                gl::bind_texture(lightbulb));
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        //TBN_shader.use();
        //TBN_shader.set_uniform<GL_FLOAT_MAT4> ("u_model", &model);
        //TBN_shader.set_uniform<GL_FLOAT_MAT4> ("u_mvp", &mvp);
        //TBN_shader.set_uniform<GL_FLOAT_VEC3> ("u_cam_pos", &cam_pos);
        //gfx::render_span(&ctx->meshes[mesh_index], span_index, true);

        glEnable(GL_DEPTH_TEST);
    }
}



