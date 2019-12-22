#version 450

in struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

uniform sampler2D u_tex;
uniform float u_time;

out vec4 frag_color;

void main() {
    float segs = 11.0;
    vec2 uv = floor(frag_in.uv * segs) / segs;

    frag_color = texture(u_tex, frag_in.uv);
//    frag_color.w = 1.0;
}
