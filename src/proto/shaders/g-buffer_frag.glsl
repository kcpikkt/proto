#version 450
layout(location = 0) out vec4 g_position;
layout(location = 1) out vec4 g_normal_shin;
layout(location = 2) out vec4 g_albedo_spec;

in struct VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
} frag_in;

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

void main() {
    g_position = vec4(frag_in.position, 1.0);
    g_normal_shin.rgb = vec4(normalize(frag_in.normal), 1.0).rgb;
    g_normal_shin.a = u_material.shininess;
    //g_albedo_spec.rgb = texture(u_material.diffuse_map, frag_in.uv).rgb;
    g_albedo_spec.rgb = texture(u_material.diffuse_map, frag_in.uv).rgb;
    g_albedo_spec.a = texture(u_material.specular_map, frag_in.uv).r;
    
}
