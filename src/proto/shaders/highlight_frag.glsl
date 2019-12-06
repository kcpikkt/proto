#version 450 core

in VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec4 dirlight_space_pos;
} frag_in;

uniform float u_time;
uniform mat4 u_mvp;
uniform mat4 u_model;
uniform mat4 u_dirlight_matrix;
uniform vec3 u_cam_position;
uniform vec4 u_highlight_color;
out vec4 frag_color;

void main() {
    frag_color = u_highlight_color;
}


