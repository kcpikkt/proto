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
