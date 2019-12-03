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

    float dirlight_shadow() {
        vec3 normal = normalize(frag_in.normal);
        vec3 to_dirlight_dir = normalize(-u_dirlight[0].direction);
        vec3 proj_coords = 
            frag_in.dirlight_space_pos.xyz / frag_in.dirlight_space_pos.w;
        proj_coords = proj_coords * 0.5 + 0.5;
        float dirlight_depth = texture(u_dirlight[0].shadow_map, proj_coords.xy).r;
        float cam_depth = proj_coords.z;
        float bias = max(0.02 * (1.0 - dot(normal,to_dirlight_dir)), 0.0006);
        float shadow = (cam_depth-bias) > dirlight_depth ? 0.9 : 0.0;
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

float global_ambient = 0.1;
vec3 light_color = vec3(1.0,1.0,1.0);

// PARALAX MAPPING

mat3 TBN = transpose(mat3(
    normalize(frag_in.tangent), 
    normalize(frag_in.bitangent), 
    normalize(frag_in.normal) ));

vec3 frag_pos = (u_model * vec4(frag_in.position, 1.0)).xyz;
vec3 tan_cam_dir = normalize(TBN * u_cam_pos - TBN * frag_pos);

float paralax =  0.01;

float height = texture(u_material.bump_map, frag_in.uv).r;    
vec2 p = tan_cam_dir.xy / tan_cam_dir.z * (height * paralax);
vec2 uv = frag_in.uv - p;    

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
shadow = 1.0;
            
frag_color = vec4((frag_diffuse + frag_specular) * (shadow) + frag_ambient , 1.0);
}

