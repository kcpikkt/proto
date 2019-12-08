#version 450
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

out struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

uniform float u_time;
uniform mat4 u_mvp;
uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main() {
    frag_in.position = (u_model * vec4(a_position, 1.0)).xyz;
    // KNOWLEDGE DEBT
    mat3 normal_matrix = transpose(inverse(mat3(u_model)));
    frag_in.normal = normal_matrix * a_normal;

    frag_in.uv = a_uv * vec2(1.0,-1.0);

    gl_Position = u_mvp * vec4(a_position, 1.0);
}

