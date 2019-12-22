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
uniform mat3 u_matrix;

void main() {
    frag_in.position = a_position;
    frag_in.normal = a_normal; 
    frag_in.uv = a_uv;

    vec3 pos = u_matrix * vec3(a_position.xy, 1.0);
    gl_Position = vec4(pos.x, pos.y, 0.0, 1.0);
}
 


