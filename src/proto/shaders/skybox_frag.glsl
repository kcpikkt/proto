#version 450
in vec3 tex_coords;

in struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

uniform samplerCube u_skybox;
out vec4 frag_color;

void main() {
    frag_color = texture(u_skybox, frag_in.position);
}
