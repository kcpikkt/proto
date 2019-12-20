#version 450

in struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

uniform struct GBuffer {
    sampler2D normal;
} gbuf;



uniform sampler2D u_tex;
uniform sampler2D u_bloom;
uniform vec2 u_resolution;
uniform float u_time;

out vec4 frag_color;


void main() {

    vec4 g_normal_shin   = texture(gbuf.normal, frag_in.uv);
    vec3 g_normal        = g_normal_shin.rgb;

    if(length(g_normal) < 0.9) {
        frag_color = vec4(0.0); return;}

    const float gamma = 1.6;
    const float exposure = 1.0;
    vec3 hdr_color = texture(u_tex, frag_in.uv).rgb;
    vec3 bloom = texture(u_bloom, frag_in.uv).rgb;
    hdr_color += bloom;
  
    vec3 mapped = vec3(1.0) - exp(-hdr_color * exposure);
    mapped = pow(mapped, vec3(1.0 / gamma));

    frag_color = vec4(mapped, 1.0);
}

