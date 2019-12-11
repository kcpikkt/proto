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
    float gamma = 2.2;
    float segs = 11.0;
    float val = floor(frag_in.uv.x * segs) / segs;
    val = frag_in.uv.x;

    frag_color = vec4(vec3(val), 1.0);
}
