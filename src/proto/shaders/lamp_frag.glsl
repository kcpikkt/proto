#version 450 core
in vec4 frag_pos;

uniform float u_far;

void main() {
    gl_FragDepth = length(frag_pos.xyz) / u_far;
}
