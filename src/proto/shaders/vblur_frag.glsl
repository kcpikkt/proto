#version 450

in struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

uniform vec2 u_resolution;
uniform mat4 u_projection;
uniform sampler2D u_tex;

uniform int u_size; // max 6
out vec4 frag_color;

float kernel[7] =
    {0.245351, 0.203089, 0.115171, 0.044734, 0.011896, 0.002164, 0.000269};

void main() {
    vec4 acc = vec4(0.0);
    float pix = 1.0 / u_resolution.y;

    for(int i=(-u_size); i<u_size; i++) 
        acc += texture(u_tex, vec2(frag_in.uv.x,
                                   frag_in.uv.y + float(i) * pix))
               * kernel[abs(i)];

    frag_color = vec4(acc.rgb, 1.0);
}
