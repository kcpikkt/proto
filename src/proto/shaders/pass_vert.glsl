#version 450
layout(location = 0) in vec3 a_position;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_uv;

out struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

void main() {
    frag_in.position = a_position;
    frag_in.normal = a_normal; 
    frag_in.uv = a_uv;

    gl_Position = vec4(a_position, 1.0);
}
 

