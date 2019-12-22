#version 450 core
layout (location = 0) in vec3 a_position;

uniform mat4 u_model;
uniform mat4 u_view;

void main() {
    gl_Position = u_model * vec4(a_position,1.0);
}
