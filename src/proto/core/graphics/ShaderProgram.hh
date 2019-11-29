#pragma once
#include "proto/core/graphics/common.hh"

namespace proto {
namespace graphics {

enum ShaderType {
    Vertex = 0, Vert = 0,
    Fragment = 1, Frag = 1,
    Geometry = 2, Geom = 2,
    TesselationControl = 3, TessCtrl = 3,
    TesselationEvaluation = 4, TessEval = 4,
    Compute = 5, Comp = 5
};
    
struct ShaderProgram {
    GLuint _program;
    GLuint shaders[6] = {};

    void init();
    void set_shader_source(ShaderType type, const char * src);
    void compile_shader(ShaderType type);
    void attach_shader(ShaderType type);
    void create_shader(ShaderType type, const char * src);
    void link();
    void use();

    template<GLenum UniformType, typename UniformValueType>
    void set_uniform(const char * name, UniformValueType value );
};

} // namespace graphics
} // namespace proto 


