#version 450

in struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

uniform vec2 u_resolution;
uniform sampler2D u_tex;
uniform int u_mode;
uniform int u_size;
uniform float u_spread;

out vec4 frag_color;


float kernel[7] =
    {0.245351, 0.203089, 0.115171, 0.044734, 0.011896, 0.002164, 0.000269};

void main() {
    vec4 acc = vec4(0.0);
    vec2 pix = vec2(u_spread) / u_resolution;
 
    if(u_mode == 0) {
        for(int i=(-u_size); i<u_size; i++) {

            acc += texture(u_tex, vec2(frag_in.uv.x + float(i) * pix.x,
                                       frag_in.uv.y))
                * kernel[abs(i)];
        }
    } else {
        for(int i=(-u_size); i<u_size; i++) {

            acc += texture(u_tex, vec2(frag_in.uv.x,
                                       frag_in.uv.y + float(i) * pix.y))
                * kernel[abs(i)];
        }
    }
    frag_color = vec4(acc.rgb, 1.0);
}
