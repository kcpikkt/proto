#version 450

in struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

uniform float u_time;
uniform mat4 u_mvp;
uniform mat4 u_model;

out vec4 frag_color;

void main() {
    float dist = length(frag_in.position - vec3(u_model[3]));
    frag_color = vec4(frag_in.normal,1.0);
}
