#version 450
layout(location = 0) out vec4 frag_color;

in struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

uniform struct GBuffer {
    sampler2D position;
    sampler2D normal;
    sampler2D albedo_spec;
    sampler2D depth;
    sampler2D ssao;
} gbuf;

uniform struct DirectionalLight {
    mat4 matrix;
    sampler2D shadow_map;
    float far;
    vec3 direction;
    vec3 color;
    float intensity;
} u_dirlight[1];

const int pointlight_count = 3;
uniform struct PointLight {
    vec3 position;
    vec3 color;
    samplerCube shadow_map;
    float far;
    float intensity;
    int on;
} u_pointlight[pointlight_count];

uniform float u_time;
uniform float u_cam_far;
uniform float u_cam_near;
uniform mat4 u_model;
uniform mat4 u_cam_pos;

float bias = 0.001;
float normal_offset = 0.05;
vec4 dirlight_space_frag_pos;
vec3 dirlight_proj_coords;

float angle_falloff;
//tmps
float shadow;
vec3 light;
vec3 diffuse;
vec3 specular;

vec3 frag_diffuse;
vec3 frag_specular;
vec3 frag_ambient;

#define DIRLIGHT_SHADOW 1

void main() {
    // gbuffer
    vec3 g_frag_pos      = texture(gbuf.position, frag_in.uv).rgb;

    vec4 g_normal_shin   = texture(gbuf.normal, frag_in.uv);
    vec3 g_normal        = g_normal_shin.rgb;
    float g_shininess    = g_normal_shin.a;

    if(length(g_normal) < 0.9) {
        frag_color = vec4(0.0);
        return;
    }

    vec4 g_albedo_spec   = texture(gbuf.albedo_spec, frag_in.uv);
    vec3 g_albedo        = g_albedo_spec.rgb;
    float g_specular     = g_albedo_spec.a;

    float g_depth        = texture(gbuf.depth, frag_in.uv).r;

    float linear_depth =
        (2.0f * u_cam_near * u_cam_far) /
        (u_cam_far + u_cam_near - (g_depth * 2.0 - 1.0) * (u_cam_far - u_cam_near))
        / u_cam_far;
        

    float g_ssao         = texture(gbuf.ssao, frag_in.uv).r;

    frag_diffuse = vec3(0.0);
    frag_specular = vec3(0.0);
    frag_ambient = vec3(0.0);

    vec3 frag2cam = normalize(vec3(0.0) - g_frag_pos);

    float dirlight_shadow = 0.0;
#if DIRLIGHT_SHADOW
    vec3 to_dirlight_dir = normalize(-u_dirlight[0].direction);
    vec4 ls_position = u_dirlight[0].matrix * vec4(g_frag_pos, 1.0);
    vec3 proj_coords = (ls_position.xyz / ls_position.w) * 0.5 + 0.5;
    float dirlight_depth = texture(u_dirlight[0].shadow_map, proj_coords.xy).r;

    float dirlight_bias = 0.002;
    // ?
    //vec2 texel_size = 10.0 / textureSize(u_dirlight[0].shadow_map, 0);

    //for(int i=-1; i<=1; i++) { for(int j=-1; j<=1; j++) {
    //    float pcf = texture(u_dirlight[0].shadow_map,
    //                     proj_coords.xy + vec2(i,j) * texel_size).r;
    //    value += (proj_coords.z - bias > pcf) ? 0.0 : 1.0;
    //}}
    //value /= 9.0f;

    dirlight_shadow =
        (proj_coords.z - dirlight_bias) < dirlight_depth ? 1.0 : 0.0;
#endif

    shadow = 1.0 - dirlight_shadow;
    light = u_dirlight[0].color * u_dirlight[0].intensity * g_ssao ;

    angle_falloff = clamp(dot(g_normal, -u_dirlight[0].direction), 0.0, 1.0);

    diffuse = light * angle_falloff * g_albedo;

    angle_falloff = dot(reflect(normalize(-u_dirlight[0].direction),
                                g_normal), -frag2cam);
    angle_falloff = clamp(angle_falloff, 0.0, 1.0);
    angle_falloff = pow(angle_falloff, max(1.0, g_shininess));

    diffuse *= (1.0 - angle_falloff);

    frag_diffuse += diffuse;
    frag_specular += light * angle_falloff * g_specular;

    frag_ambient += g_albedo * 0.2 * (g_ssao);

    frag_color = vec4(frag_diffuse + frag_specular + frag_ambient, 1.0);
//    frag_color = vec4(vec3(dirlight_shadow), 1.0);

    #if 0
    int index = (gl_FragCoord.x / u_resolution.x)
    vec3 frag2cam = normalize(u_cam_pos - g_frag_pos);
    frag_ambient = g_albedo * 0.14;
    // DIRECTIONAL LIGHT
    dirlight_space_frag_pos = (u_dirlight[0].matrix * vec4(g_frag_pos,1.0));
    dirlight_proj_coords = dirlight_space_frag_pos.xyz / dirlight_space_frag_pos.w;
    dirlight_proj_coords = dirlight_proj_coords * 0.5 + 0.5;
    frag_depth = dirlight_proj_coords.z;

    shadowmap_depth = texture(u_dirlight[0].shadow_map,
                              dirlight_proj_coords.xy).r;

    if(frag_depth - bias < shadowmap_depth) {
        angle_falloff = clamp(dot(g_normal, -u_dirlight[0].direction), 0.0, 1.0);

        light = u_dirlight[0].color * u_dirlight[0].intensity;

        diffuse = light * angle_falloff * g_albedo;

        angle_falloff = clamp(dot(g_normal, -u_dirlight[0].direction), 0.0, 1.0);

        angle_falloff = dot(reflect(normalize(-u_dirlight[0].direction),
                                g_normal), -frag2cam);
        angle_falloff = clamp(angle_falloff, 0.0, 1.0);
        angle_falloff = pow(angle_falloff, max(1.0,u_material.shininess));

        diffuse *= (1.0 - angle_falloff);

        specular = light * angle_falloff * g_specular;

        frag_diffuse += diffuse;
        frag_specular += specular;
    }

    // POINT LIGHTS
    for(int i=0; i<pointlight_count-2; i++) {
        if(u_pointlight[i].on != 0) {
            frag2light = u_pointlight[i].position -
                          (g_frag_pos + g_normal * normal_offset);

            shadowmap_depth = texture(u_pointlight[i].shadow_map,
                                      -frag2light).r * u_pointlight[i].far;

            
            if(length(frag2light) > shadowmap_depth) continue;
            // diffuse angle factor
            angle_falloff = clamp(dot(g_normal, frag2light), 0.0, 1.0);

            frag2light_dist = length(frag2light);
            light_intensity = u_pointlight[i].intensity / pow(frag2light_dist, 2.0);
            light = light_intensity * u_pointlight[i].color;
            diffuse = light * angle_falloff * g_albedo;
            // specular angle factor
            
            angle_falloff = dot(reflect(normalize(frag2light), g_normal), -frag2cam);
            angle_falloff = clamp(angle_falloff, 0.0, 1.0);
            angle_falloff = pow(angle_falloff, max(1.0,u_material.shininess));

            diffuse *= (1.0 - angle_falloff);

            specular = light * angle_falloff * g_specular;

            frag_diffuse += diffuse;
            frag_specular += specular;
        }
    }

    frag2light = u_pointlight[0].position - g_frag_pos;

    frag_color = vec4(frag_diffuse + frag_specular + frag_ambient , 1.0);
    #endif
}
