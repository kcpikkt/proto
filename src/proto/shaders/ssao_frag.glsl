#version 450

in struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

uniform struct GBuffer {
    sampler2D position;
    sampler2D normal;
    sampler2D albedo_spec;
} gbuf;

//uniform struct Material {
//    vec3 diffuse;
//    vec3 ambient;
//    vec3 specular;
//    float shininess;
//    sampler2D diffuse_map;
//    sampler2D ambient_map;
//    sampler2D specular_map;
//    sampler2D bump_map;
//} u_material;

uniform float u_time;
uniform vec3 u_cam_pos;
uniform mat4 u_model;

//out vec4 frag_color;

void main() {}

