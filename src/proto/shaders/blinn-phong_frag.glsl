#version 450 core

in GeomOut {
    vec3 position;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 uv;
    vec4 dirlight_space_pos;
} frag_in;

uniform float u_time;
uniform mat4 u_mvp;
uniform mat4 u_model;
uniform vec3 u_cam_pos;
uniform vec2 u_resolution;

uniform uint u_flip_uv;
uniform uint u_hash;
uniform uint u_span_index;

layout(std430, binding = 0) buffer center_mesh_ssbo {
    uint center_mesh_hash;
    uint center_mesh_span_index;
    float center_mesh_depth;
};

uniform struct Material {
    vec3 diffuse;
    vec3 ambient;
    vec3 specular;
    float shininess;
    sampler2D diffuse_map;
    sampler2D ambient_map;
    sampler2D specular_map;
    sampler2D bump_map;
} u_material;

uniform struct DirectionalLight {
    sampler2D shadow_map;
    vec3 direction;
} u_dirlight[1];

uniform struct PointLight {
    vec3 position;
    float far;
    float near;
    samplerCube shadow_map;
} u_pointlight[1];

void shadow_map_smooth_sample(vec2 coords, int kernel_size) {
    float value = 0.0;
    vec2 texelsize = 1.0 / textureSize(u_dirlight[0].shadow_map, 0);
}

float cubemap_sample() {
    vec3 light2frag = frag_in.position - u_pointlight[0].position ;
    float curr_depth = length(light2frag);
    float value = 0.0;
    //float bias = 0.05;

    float bias_force = // adaptive bias, meh, works but nothing special
        1.0 - abs(dot(normalize(frag_in.normal), normalize(-light2frag)));
    float bias = bias_force * 0.5 + 0.01;

    float samples = 4.0;
    float offset = 0.1;
    samples *= 0.5;

    float init_depth =
        texture(u_pointlight[0].shadow_map,
                light2frag).r * u_pointlight[0].far;

    if(init_depth < 20.0) {
    for(float x=-offset; x < offset; x += offset/samples){
    for(float y=-offset; y < offset; y += offset/samples){
    for(float z=-offset; z < offset; z += offset/samples){
        float depth = texture(u_pointlight[0].shadow_map,
                              light2frag + vec3(x,y,z)).r;
        depth *= u_pointlight[0].far;
        if(curr_depth - bias > depth) value += 1.0;
    }}}
    return value / (samples * samples * samples);
    } else {
        return 1.0;
    }
}
float pointlight_shadow() {
    vec3 light2frag = frag_in.position - u_pointlight[0].position ;
    float texel = cubemap_sample();
        //texture(u_pointlight[0].shadow_map, light2frag).r;

    //float bias_force = // adaptive bias, meh, works but nothing special
    //    1.0 - abs(dot(normalize(frag_in.normal), normalize(-light2frag)));
    //float bias = bias_force * 0.5 + 0.01;
    //return (length(light2frag) - bias < texel * u_pointlight[0].far ? 0.0 : 1.0);
    return texel;
}

float dirlight_shadow() {
    vec3 normal = normalize(frag_in.normal);
    vec3 to_dirlight_dir = normalize(-u_dirlight[0].direction);
    vec3 proj_coords = 
        frag_in.dirlight_space_pos.xyz / frag_in.dirlight_space_pos.w;
    proj_coords = proj_coords * 0.5 + 0.5;
    //float dirlight_depth = texture(u_dirlight[0].shadow_map, proj_coords.xy).r;
    //float dirlight_depth = shadow_map_smooth_sample(proj_coords.xy, 1);
    float cam_depth = proj_coords.z;
    float bias = 0.002;

    //float value = texture(u_dirlight[0].shadow_map, proj_coords.xy).r;
    float value = 0.0;
    vec2 texel_size = 10.0 / textureSize(u_dirlight[0].shadow_map, 0);
    for(int i=-1; i<=1; i++) { for(int j=-1; j<=1; j++) {
        float pcf = texture(u_dirlight[0].shadow_map,
                         proj_coords.xy + vec2(i,j) * texel_size).r;
        value += (proj_coords.z - bias > pcf) ? 0.0 : 1.0;
    }}
    value /= 9.0f;

    float shadow = (cam_depth-bias) > value ? 0.9 : 0.0;
    return shadow;
}

out vec4 frag_color;

void main() {

    // CENTER MESH SPAN HIGHTLIGHT

    vec2 half_res = u_resolution/2.0;
    float cursor_sz = 1.0;

    if(gl_FragCoord.x > half_res.x             &&
    gl_FragCoord.x < half_res.x + cursor_sz &&
    gl_FragCoord.y > half_res.y             &&
    gl_FragCoord.y < half_res.y + cursor_sz ) 
    {
    if(center_mesh_depth >= gl_FragCoord.z) {
        atomicExchange(center_mesh_hash, u_hash);
        atomicExchange(center_mesh_span_index, u_span_index);
        center_mesh_depth = gl_FragCoord.z;
    }
    discard;
    }

    float global_ambient = 0.2;
    vec3 light_color = vec3(1.0,1.0,1.0);

    // PARALAX MAPPING

    mat3 TBN = transpose(mat3(
        normalize(frag_in.tangent), 
        normalize(frag_in.bitangent), 
        normalize(frag_in.normal) ));

    vec3 frag_pos = (u_model * vec4(frag_in.position, 1.0)).xyz;
    vec3 tan_cam_dir = normalize(TBN * u_cam_pos - TBN * frag_pos);

    float paralax =  0.01;

    vec2 uv = (frag_in.uv) * vec2(1.0,-1.0);    
    float height = texture(u_material.bump_map, uv).r;    
    vec2 p = tan_cam_dir.xy / tan_cam_dir.z * (height * paralax);
    uv = (uv - p);    

    // PHONG

    float angle_factor;
    vec3 frag_ambient;
    vec3 frag_specular;
    vec3 frag_diffuse;

    frag_ambient = 
        global_ambient * 
        u_material.ambient * 
        texture(u_material.ambient_map, uv).xyz;

    vec3 normal = normalize(frag_in.normal);
    vec3 to_dirlight_dir = normalize(-u_dirlight[0].direction);

    angle_factor = clamp(dot(normal,to_dirlight_dir), 0.0, 1.0);

    frag_diffuse = 
        angle_factor *
        u_material.diffuse *
        light_color *
        texture(u_material.diffuse_map, uv).xyz;

    vec3 to_cam_dir = normalize(u_cam_pos - frag_in.position);
    angle_factor = dot(reflect(-to_dirlight_dir, normal), to_cam_dir);
    angle_factor = clamp(angle_factor, 0.0, 1.0);
    angle_factor = pow(angle_factor, max(1.0,u_material.shininess));

    float global_specular = 5.0f;
    frag_specular =
        global_specular *
        angle_factor *
        light_color *
        u_material.specular *
        texture(u_material.specular_map, uv).xyz;

    frag_diffuse *= (1.0 - angle_factor);

    float shadow = clamp(1.0 - dirlight_shadow(), 0.0, 1.0);
    //shadow = 1.0 - pointlight_shadow();// * dirlight_shadow();

    frag_color = vec4((frag_diffuse + frag_specular) * (shadow) +
                      frag_ambient , 1.0);

    vec3 light2frag = frag_in.position - u_pointlight[0].position ;
    //frag_color = vec4(vec3(bias_force),1.0);
}

