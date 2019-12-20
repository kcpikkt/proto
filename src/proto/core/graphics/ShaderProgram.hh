#pragma once
#include "proto/core/graphics/common.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/asset-system/common.hh"

namespace proto {

struct Material;

enum ShaderType {
    Vert = 0,
    Frag = 1,
    Geom = 2,
    TessCtrl = 3,
    TessEval = 4,
    Comp = 5
};
    
struct ShaderProgram : Asset {
    u32 _program;
    u32 shaders[6] = {};

    void init();
    void set_shader_source(ShaderType type, const char * src);
    void compile_shader(ShaderType type);

    void attach_shader_file(ShaderType type, StringView path);
    void attach_shader_src(ShaderType type, const char * src);

    void create_shader(ShaderType type, const char * src);
    void create_shader_from_file(ShaderType type, StringView path);
    void link();
    void use();

    void set_material(Material * value );

    template<GLenum UniformType, typename UniformValueType>
    void set_uniform(const char * name, UniformValueType value );

    void set_int   (const char*, s32);
    void set_float (const char*, float);
    void set_vec2  (const char*, vec2*);
    void set_vec3  (const char*, vec3*);
    void set_mat3  (const char*, mat3*);
    void set_mat4  (const char*, mat4*);
    void set_tex2D (const char*, s32);
    void set_tex2D (const char*, AssetHandle);

    //chaining
    inline ShaderProgram& $_attach_shader_file(ShaderType type, StringView path) {
        attach_shader_file(type, path); return *this;
    }

    inline ShaderProgram& $_attach_shader_src(ShaderType type, const char * src) {
        attach_shader_src(type, src); return *this;
    }

    inline ShaderProgram& $_link() {
        link(); return *this;
    }

    inline ShaderProgram& $_use() {
        use(); return *this;
    }

    template<GLenum UniformType, typename UniformValueType>
    inline ShaderProgram& $_set_uniform(const char * name, UniformValueType value ) {
        set_uniform<UniformType, UniformValueType>(name, value); return *this;
    }

    ShaderProgram& $_set_int   (const char*, s32);
    ShaderProgram& $_set_float (const char*, float);
    ShaderProgram& $_set_vec2  (const char*, vec2*);
    ShaderProgram& $_set_vec3  (const char*, vec3*);
    ShaderProgram& $_set_mat3  (const char*, mat3*);
    ShaderProgram& $_set_mat4  (const char*, mat4*);
    ShaderProgram& $_set_tex2D (const char*, s32);
    ShaderProgram& $_set_tex2D (const char*, AssetHandle);
};

} // namespace proto 


