#version 450
layout(location = 0) out vec4 out_ssao;

in struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

uniform struct GBuffer {
    sampler2D position;
    sampler2D normal;
    sampler2D depth;
} gbuf;

#define clamp01(val) clamp(val, 0.0, 1.0)
#define KERNEL_SZ 64
uniform vec3 kernel[64];

uniform vec2 u_resolution;
uniform float u_time;
uniform mat4 u_projection;
uniform mat4 u_view;
uniform sampler2D u_noise;
uniform float u_radius;
float bias = 0.025;

void main() {
    vec2 noise_uv =   // change this after going fullres
        frag_in.uv * (1.0f * u_resolution / textureSize(u_noise, 0));
    vec3 normal = normalize(texture(gbuf.normal, frag_in.uv).xyz);
    vec3 position = (texture(gbuf.position, frag_in.uv)).xyz;
    float depth   = (texture(gbuf.depth, frag_in.uv)).r;

    vec3 random_vec = normalize(texture(u_noise, noise_uv).xyz);
    vec3 tangent = normalize(random_vec - normal * dot(random_vec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
    float occlussion = 0.0;

    for(int i=0; i<KERNEL_SZ ; i++) {
        vec3 kernel_smpl = (TBN * kernel[i]);

        vec4 smpl = u_view * vec4(position + kernel_smpl * u_radius, 1.0);
        vec4 ss_smpl = u_projection  * smpl;
        ss_smpl = (ss_smpl / ss_smpl.w) * 0.5 + 0.5;

        float smpl_depth = (u_view * texture(gbuf.position, ss_smpl.xy)).z;
        occlussion += (smpl_depth >= smpl.z - bias ? 1.0 : 0.0);
    }

    occlussion = 1.0 - occlussion / KERNEL_SZ;

    out_ssao = vec4(occlussion, 0.0, 0.0, 1.0);
    //if(frag_in.uv.y > 0.5)
    //    out_ssao = vec4( length(smpl), 0.0, 0.0, 1.0);
    //else
    //    out_ssao = vec4( length(ss_smpl.xyz), 0.0, 0.0, 1.0);

    #if 0
    float thres = 0.008;
    bool match = false;
    vec2 pos = gl_FragCoord.xy / u_resolution;
    float scale = 0.5;
    vec3 trans = vec3(0.5,0.0, 0.3);
    for(int i=0; i<KERNEL_SZ; i++) {
        float len = length(pos - (kernel[i]*scale + trans).xz);
        if(len < thres * ((kernel[i].y + 1.0) * 0.5)) {
            out_ssao = vec4(1.0);
            match = true;
            break;
        }
        len = length(pos - trans.xz);
        if(len > scale && len < (scale + thres)) {
            out_ssao = vec4(0.5);
            match = true;
            break;
        }
        if(len > 0.1 && len < (0.1 + thres)) {
            out_ssao = vec4(0.5);
            match = true;
            break;
        }

    }
    if(!match) {discard;}
    #endif
}
