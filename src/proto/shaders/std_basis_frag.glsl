#version 450
in struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

out vec4 frag_color;
void main() {
    frag_color = vec4(frag_in.normal, 1.0);
}
 

