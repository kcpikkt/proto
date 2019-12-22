#version 450
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

out struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

uniform struct GBuffer {
    sampler2D position;
    sampler2D normal;
    sampler2D albedo_spec;
} gbuf;

uniform float u_time;
uniform mat4 u_mvp;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    frag_in.position = a_position;
    frag_in.normal = a_normal;
    // idk, uvs are screwed
    frag_in.uv = (a_position.xy + vec2(1.0)) * 0.5;

    gl_Position = vec4(a_position, 1.0);
}
 

