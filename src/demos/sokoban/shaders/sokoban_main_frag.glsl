#version 450

in struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

layout (std140) uniform Material {
    float a;
    float b;
};

uniform float u_time;
uniform mat4 u_mvp;
uniform mat4 u_model;
uniform vec3 u_color;

out vec4 frag_color;

void main() {
    frag_color = vec4(abs(frag_in.normal) , 1.0);
}
