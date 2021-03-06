#version 450

in struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

uniform float u_cam_far;
uniform float u_cam_near;
uniform vec2 u_resolution;
uniform mat4 u_projection;
uniform sampler2D u_tex;
uniform sampler2D u_depth;

uniform float u_time;
uniform int u_mode;
uniform int u_size;
out vec4 frag_color;

float kernel[7] =
    {0.245351, 0.203089, 0.115171, 0.044734, 0.011896, 0.002164, 0.000269};

float linearize_depth(float z)  {
    return (2.0f * u_cam_near * u_cam_far) /
    (u_cam_far + u_cam_near - (z * 2.0 - 1.0) * (u_cam_far - u_cam_near));
}

void main() {
    float thres = 0.1;
    vec4 acc = vec4(0.0);
    float pix = 1.0 / u_resolution.x;
    float depth        = linearize_depth(texture(u_depth, frag_in.uv).r);
 
    vec2 smpl_uv;
    float smpl_depth;
    vec4 basepix = texture(u_tex, frag_in.uv);

    #if 1

    if(u_mode == 0) {
        for(int i=(-u_size); i<u_size; i++) {
            smpl_uv = vec2(frag_in.uv.x + float(i) * pix, frag_in.uv.y);
            smpl_depth = linearize_depth(texture(u_depth, smpl_uv).r);

            acc += (abs(smpl_depth - depth) > thres 
                    ? basepix
                    : texture(u_tex, smpl_uv) )
                * kernel[abs(i)];
        }
    } else {
        for(int i=(-u_size); i<u_size; i++) {
            smpl_uv = vec2(frag_in.uv.x, frag_in.uv.y + float(i) * pix);
            smpl_depth = linearize_depth(texture(u_depth, smpl_uv).r);

            acc += (abs(smpl_depth - depth) > thres 
                    ? basepix
                    : texture(u_tex, smpl_uv) )
                * kernel[abs(i)];
        }
    }
    #endif
    frag_color = vec4(acc.rgb, 1.0);
}
