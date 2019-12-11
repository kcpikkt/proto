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

    frag_color = vec4(uv.x, 0.0, uv.y, 1.0);
    frag_color = texture(u_tex, frag_in.uv);
}
