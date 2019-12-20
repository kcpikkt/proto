#version 450 core
layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in VertOut {
    vec3 position;
    vec3 normal;
    vec2 uv;
    vec4 dirlight_space_pos;
} geom_in[];

out GeomOut {
    vec3 position;
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
    vec2 uv;
    vec4 dirlight_space_pos;
} frag_in;

uniform mat4 u_mvp;
uniform float u_time;


mat2x3 computeTB(int index) {

    int v0 = (index + 0)%3; 
    int v1 = (index + 1)%3; 
    int v2 = (index + 2)%3; 
    vec3 e1 = (geom_in[v1].position - geom_in[v0].position); // edge between
    vec3 e2 = (geom_in[v2].position - geom_in[v0].position);

    vec2 d1 = (geom_in[v1].uv - geom_in[v0].uv); // uv deltas
    vec2 d2 = (geom_in[v2].uv - geom_in[v0].uv);
    
    float inv_det = 1.0/(d1.x * d2.y - d1.y * d2.x);

    mat2x2 adj = {{-d2.y, d2.x}, { d1.y,-d1.x}};
    mat3x2 E = {{ e1.x, e2.x}, { e1.y, e2.y}, { e1.z, e2.z}};

    return transpose(inv_det * adj * E);
}

void emit(int index) {
    mat2x3 TB = computeTB(index);
    frag_in.position           = geom_in[index].position;
    frag_in.normal             = geom_in[index].normal;
    frag_in.tangent            = normalize(TB[0]);
    frag_in.bitangent          = normalize(TB[1]);
    frag_in.uv                 = geom_in[index].uv;
    frag_in.dirlight_space_pos = geom_in[index].dirlight_space_pos;
    gl_Position = gl_in[index].gl_Position; 
    EmitVertex();
}

void main() {
    int i; for(i=0; i<gl_in.length(); i++) {emit(i);}
    EndPrimitive();
}
