#version 450
layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec4 frag_bloom;

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
uniform mat4 u_projection;
uniform mat4 u_view;
uniform vec3 u_cam_pos;

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
#define ANTIALIAS 0
#define VOLUMETRIC 1
#define SSR 0

#define ALIAS_THRES 0.15


#if VOLUMETRIC
// super slow volumetric light fog, purely experimental

// based on 
// https://thebookofshaders.com/13/
float random (in vec3 _st) {
    return fract(sin(dot(_st.xyz,
                         vec3(12.9898,78.233,881.12)))*
        43758.5453123);
}

float noise (in vec3 _st) {
    vec3 i = floor(_st);
    vec3 f = fract(_st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec3(1.0, 0.0, 1.0));
    float c = random(i + vec3(0.0, 1.0, 0.0));
    float d = random(i + vec3(1.0, 1.0, 1.0));

    vec3 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

#define NUM_OCTAVES 3

float fbm ( in vec3 _st) {
    float v = 0.0;
    float a = 0.5;
    vec3 shift = vec3(100.0);
    mat3 rot = mat3(cos(0.5), sin(0.5), 0.0,
                    -sin(0.5), cos(0.50), 0.0,
                    0.0,0.0,0.0);

    for (int i = 0; i < NUM_OCTAVES; ++i) {
        v += a * noise(_st);
        _st = rot * _st * 2.0 + shift;
        a *= 0.5;
    }
    return v;
}

float vol_fog_noise(vec3 pos) {
    vec3 st = pos.xzy * 3.0;
   
    vec3 q = vec3(0.0);
    q.x = fbm( st + 0.21*u_time );
    q.y = fbm( st + vec3(1.0));
    q.z = fbm( st + u_time );

    vec3 r = vec3(0.0);
    r.x = fbm( st + 1.0*q + vec3(1.7,9.2,9.4)+ 0.15 *u_time);
    r.y = fbm( st + 0.0*q + vec3(8.3,2.8,6.0)+ 0.126*u_time);
    r.z = fbm( st + 1.0*q + vec3(1.3,8.8,2.4)+ 0.182*u_time);

    return pow(fbm(st+r), 2.0);
}
#endif

void main() {
    // gbuffer

    vec3 g_frag_pos      = texture(gbuf.position, frag_in.uv).rgb;

    vec4 g_normal_shin   = texture(gbuf.normal, frag_in.uv);
    vec3 g_normal        = g_normal_shin.rgb;
    float g_shininess    = g_normal_shin.a;

   
    vec4 g_albedo_spec   = texture(gbuf.albedo_spec, frag_in.uv);
    vec3 g_albedo        = g_albedo_spec.rgb;
    float g_specular     = g_albedo_spec.a;

    if(g_shininess > 1000.0) {
        g_albedo *= 40.0;
    }
    
    float g_depth        = texture(gbuf.depth, frag_in.uv).r;

    float linear_depth =
        (2.0f * u_cam_near * u_cam_far) /
        (u_cam_far + u_cam_near - (g_depth * 2.0 - 1.0) * (u_cam_far - u_cam_near))
        / u_cam_far;

    vec3 frag2cam = u_cam_pos - g_frag_pos;
    vec3 frag2cam_norm = normalize(frag2cam);

#if ANTIALIAS

    // complex pixel testing "antialiasing", better than nothing...
    // https://docs.nvidia.com/gameworks/content/gameworkslibrary/graphicssamples/d3d_samples/antialiaseddeferredrendering.htm
    vec2 pix = vec2(1.0) / textureSize(gbuf.normal, 0);

    vec2 alias_pix[4] = {vec2( pix.x, 0.0), vec2(-pix.x, 0.0),
                         vec2( 0.0, pix.y), vec2( 0.0,-pix.y)};

    vec3 real_albedo = g_albedo;
    for(int i=0; i<4; i++) {
        vec3 neighb_normal = texture(gbuf.normal, frag_in.uv + alias_pix[i]).rgb;
        if(length(g_normal - neighb_normal) > ALIAS_THRES) {
            g_albedo += texture(gbuf.albedo_spec, frag_in.uv + alias_pix[i]).rgb;
            //g_albedo = vec3(4.0,4.0,4.0);
        } else {
            g_albedo += real_albedo;
        }
    }
    g_albedo /= 4.0;
    // poor
#endif

    float g_ssao         = texture(gbuf.ssao, frag_in.uv).r;

    frag_diffuse = vec3(0.0);
    frag_specular = vec3(0.0);
    frag_ambient = vec3(0.0);

    float dirlight_shadow = 1.0;
#if DIRLIGHT_SHADOW
    vec3 to_dirlight_dir = normalize(-u_dirlight[0].direction);
    vec4 ls_position = u_dirlight[0].matrix * vec4(g_frag_pos, 1.0);
    vec3 proj_coords = (ls_position.xyz / ls_position.w) * 0.5 + 0.5;
    float dirlight_depth = texture(u_dirlight[0].shadow_map, proj_coords.xy).r;

    float dirlight_bias = 0.002;

    vec2 pix = 1.0 / textureSize(u_dirlight[0].shadow_map, 0);
    for(int x = -1; x <= 1; ++x) {
        for(int y = -1; y <= 1; ++y) {
            float pcf = texture(u_dirlight[0].shadow_map,
                                proj_coords.xy + vec2(x, y) * pix).r; 

            dirlight_shadow +=
                proj_coords.z - dirlight_bias > pcf ? 1.0 : 0.0;        
        }
    }    
    dirlight_shadow = 1.0 - dirlight_shadow / 9.0;
    //dirlight_shadow =
    //    (proj_coords.z - dirlight_bias) < dirlight_depth ? 1.0 : 0.0;
    //


float dirlight_vol = 0.0;
#if VOLUMETRIC
    float march_step = 0.04;

    float frag2cam_dist = length(frag2cam);

    vec3 march_pos = g_frag_pos; 

    int steps = 0;
    for(steps; (steps * march_step) < frag2cam_dist; steps++) {
        march_pos = g_frag_pos + frag2cam_norm * steps * march_step;

        ls_position = u_dirlight[0].matrix * vec4(march_pos, 1.0);
        proj_coords = (ls_position.xyz / ls_position.w) * 0.5 + 0.5;
        dirlight_depth = texture(u_dirlight[0].shadow_map, proj_coords.xy).r;

        dirlight_vol +=
            (proj_coords.z - dirlight_bias) < dirlight_depth
            ? vol_fog_noise(march_pos ) : 0.0;
    }
    dirlight_vol /= (steps * 1.0);
    dirlight_vol = clamp(dirlight_vol,0.0,0.5);

#endif
#endif


    vec3 ssr = vec3(0.0);
#if SSR
    if(g_shininess > 60.0) {
        vec3 ssr_refl = reflect(frag2cam_norm, g_normal);
        vec3 ssr_step_pos = g_frag_pos;

        float ssr_step_size = 0.1;

        vec4 ssr_ss_pos;
        for(int i=0; i<100; i++) {
            vec3 ssr_pos = g_frag_pos + ssr_step_size * ssr_refl * float(i);

            ssr_ss_pos = u_projection * u_view * vec4(ssr_pos, 1.0);
            proj_coords = (ssr_ss_pos.xyz / ssr_ss_pos.w) * 0.5 + 0.5;
            float ssr_depth = proj_coords.z;
            float real_depth = texture(gbuf.depth, proj_coords.xy).r;

            float real_linear_depth =
                (2.0f * u_cam_near * u_cam_far) /
                (u_cam_far + u_cam_near -
                 (real_depth * 2.0 - 1.0) * (u_cam_far - u_cam_near))
                / u_cam_far;
     
            if(real_linear_depth < ssr_depth + 0.1) {
                ssr = texture(gbuf.albedo_spec, proj_coords.xy).rgb;
                break;
            }
        }


    }
#endif
    shadow = dirlight_shadow;
    light = u_dirlight[0].color * u_dirlight[0].intensity * g_ssao * dirlight_shadow;

    angle_falloff = clamp(dot(g_normal, -u_dirlight[0].direction), 0.0, 1.0);

    diffuse = light * angle_falloff * g_albedo;

    angle_falloff = dot(reflect(normalize(-u_dirlight[0].direction),
                                g_normal), -frag2cam_norm);
    angle_falloff = clamp(angle_falloff, 0.0, 1.0);
    angle_falloff = pow(angle_falloff, max(1.0, g_shininess));

    diffuse *= (1.0 - angle_falloff);

    frag_diffuse += diffuse;
    frag_specular += light * angle_falloff * g_specular + ssr/2.0;

    frag_ambient += g_albedo * 0.15 * (g_ssao);

    frag_ambient += dirlight_vol * u_dirlight[0].color;

    vec3 frag2light;
    vec3 frag2light_norm;
    // POINT LIGHTS
    for(int i=0; i<pointlight_count-2; i++) {
        if(u_pointlight[i].on != 0) {

            light =
                u_pointlight[i].color *
                u_pointlight[i].intensity * g_ssao;

            frag2light = u_pointlight[i].position - g_frag_pos;
            frag2light_norm = normalize(frag2light);

            angle_falloff =
                clamp(dot(g_normal, frag2light_norm), 0.0, 1.0);

            diffuse = light * angle_falloff * g_albedo / pow(length(frag2light),2.0);

            angle_falloff = dot(reflect(frag2light_norm, g_normal), -frag2cam_norm);
            angle_falloff = clamp(angle_falloff, 0.0, 1.0);
            angle_falloff = pow(angle_falloff, max(1.0, g_shininess));

            diffuse *= (1.0 - angle_falloff);

            frag_diffuse += diffuse;
            frag_specular += light * angle_falloff * g_specular + ssr/2.0;

#if 0
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
            
            angle_falloff = dot(reflect(normalize(frag2light), g_normal),
                                -frag2cam_norm);
            angle_falloff = clamp(angle_falloff, 0.0, 1.0);
            angle_falloff = pow(angle_falloff, max(1.0,g_shininess));

            diffuse *= (1.0 - angle_falloff);

            specular = light * angle_falloff * g_specular;

            frag_diffuse += diffuse;
            frag_specular += specular;
    #endif
        }
    }

    frag_color = vec4(frag_diffuse + frag_specular + frag_ambient , 1.0);
    frag_bloom = length(frag_color.xyz) > 4.2 ? frag_color : vec4(0.0);
}
