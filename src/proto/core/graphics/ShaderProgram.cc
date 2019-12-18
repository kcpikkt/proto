#include "proto/core/graphics/ShaderProgram.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/common/types.hh"
#include "proto/core/context.hh"
#include "proto/core/platform/api.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/graphics/gl.hh"
#include "proto/core/util/namespace-shorthands.hh"
#include "proto/core/util/String.hh"

namespace proto {

constexpr GLenum gl_shader_type(ShaderType type) {
    switch(type) {
    case ShaderType::Vert:     return GL_VERTEX_SHADER;
    case ShaderType::Frag:     return GL_FRAGMENT_SHADER;
    case ShaderType::Geom:     return GL_GEOMETRY_SHADER;
    case ShaderType::TessCtrl: return GL_TESS_CONTROL_SHADER;
    case ShaderType::TessEval: return GL_TESS_EVALUATION_SHADER;
    case ShaderType::Comp:     return GL_COMPUTE_SHADER;
    }
    assert(0);
    return 0;
}

//TEMP
const char * shader_type_name[] = {"Vertex", "Fragment", "Geometry"};

void ShaderProgram::init() {
    _program = glCreateProgram();
    assert_info(_program,
                proto::debug::category::graphics,
                "Failed to create shader program");
}

void ShaderProgram::set_shader_source(ShaderType type, const char * src) {
    auto& shader = shaders[(int)type];
    if(shader == 0)
        shader = glCreateShader(gl_shader_type(type));

    glShaderSource(shader, 1, &src, NULL);
}

void ShaderProgram::compile_shader(ShaderType type) {
    GLint status;
    auto& shader = shaders[(int)type];
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if(!status) {
        GLint log_length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_length);

        if(log_length) {
            //FIXME(kacper): use string allocator
            char * log_buffer = (char*)
                proto::context->memory.alloc(log_length, 16);

            assert_info(log_buffer,
                        proto::debug::category::memory | 
                        proto::debug::category::graphics,
                        shader_type_name[type], " ",
                        "shader compilation failed but was unable to allocate "
                        "error message buffer in "
                        "linked_list_allocator global_state.memory")

            glGetShaderInfoLog(shader, log_length, NULL, log_buffer);
            debug_error(proto::debug::category::graphics, (const char *)log_buffer);
        }
    }
}

void ShaderProgram::attach_shader_src(ShaderType type, const char * src) {
    shaders[(int)type] = glCreateShader(gl_shader_type(type));
    auto& shader = shaders[ (int)type ];
    
    set_shader_source(type, src);
    compile_shader(type);
    glAttachShader(_program, shader);
}

void ShaderProgram::attach_shader_file(ShaderType type, StringView path) {
    String filepath =
        platform::search_for_file(proto::context->shader_paths, path);

    if(!filepath) {
        debug_error(debug::category::graphics,
                    "Could not find shader file ", path); return; }

    platform::File file;
    assert(!file.open(filepath.view(), platform::file_read));

    memory::Allocator * allocator = &(context->memory);
    u8 * buf = (u8*)allocator->alloc(file.size() + 1);

    assert(buf);
    assert(file.size() == file.read(buf, file.size() ));

    buf[file.size()] = '\0';

    const char * src = (const char *)buf;
    attach_shader_src(type, src);

    allocator->free(buf);
}



void ShaderProgram::link() {
    GLint status;
    glLinkProgram(_program);
    glGetProgramiv(_program, GL_LINK_STATUS, &status);
    if(status == GL_FALSE) {
        GLint log_length;
        glGetProgramiv(_program, GL_INFO_LOG_LENGTH, &log_length);

        if(log_length) {
            //FIXME(kacper): use different allocator
            char * log_buffer = (char*)
                proto::context->memory.alloc(log_length, 16);

            assert_info(log_buffer || log_length == 0,
                        proto::debug::category::memory | 
                        proto::debug::category::graphics,
                        "shader program linking failed but was unable to allocate "
                        "error message buffer in "
                        "linked_list_allocator global_state.memory")

            glGetProgramInfoLog(_program, log_length, NULL, log_buffer);
            debug_error(proto::debug::category::graphics, log_buffer);
        }
    }
}

void ShaderProgram::use() {
    glUseProgram(_program);
    proto::context->current_shader = this;
}


template<>
void ShaderProgram::set_uniform<GL_SAMPLER_2D, s32>
(const char * name, s32 value ) {
    glUniform1i ( glGetUniformLocation(_program, name), (s32)value);
}

template<>
void ShaderProgram::set_uniform<GL_SAMPLER_2D, AssetHandle>
(const char * name, AssetHandle handle ) {
    // TODO(kacper): error msg bind_tex ret val
    glUniform1i ( glGetUniformLocation(_program, name),
                (s32)gfx::bind_texture(handle));
}

// -----------
template<>
void ShaderProgram::set_uniform<GL_INT, s32>
(const char * name, s32 value ) {
    glUniform1i ( glGetUniformLocation(_program, name), (s32)value);
}

// -----------
template<>
void ShaderProgram::set_uniform<GL_UNSIGNED_INT, u32>
(const char * name, u32 value ) {
    glUniform1ui ( glGetUniformLocation(_program, name), (s32)value);
}

// -----------
template<>
void ShaderProgram::set_uniform<GL_FLOAT, float>
(const char * name, float value ) {
    glUniform1f ( glGetUniformLocation(_program, name), (float)value);
}

// ------------
template<>
void ShaderProgram::set_uniform<GL_FLOAT_VEC2, float *>
(const char * name, float * value ) {
    glUniform2fv ( glGetUniformLocation(_program, name), 1, (float * )value);
}

template<>
void ShaderProgram::set_uniform<GL_FLOAT_VEC2, vec2 *>
(const char * name, vec2 * value ) {
    glUniform2fv ( glGetUniformLocation(_program, name), 1, (float * )value);

}

// ------------
template<>
void ShaderProgram::set_uniform<GL_FLOAT_VEC3, float *>
(const char * name, float * value ) {
    glUniform3fv ( glGetUniformLocation(_program, name), 1, (float * )value);
}

template<>
void ShaderProgram::set_uniform<GL_FLOAT_VEC3, vec3 *>
(const char * name, vec3 * value ) {
    glUniform3fv ( glGetUniformLocation(_program, name), 1, (float * )value);
}

// ------------
template<>
void ShaderProgram::set_uniform<GL_FLOAT_VEC4, float *>
(const char * name, float * value ) {
    glUniform4fv ( glGetUniformLocation(_program, name), 1, (float * )value);
}

template<>
void ShaderProgram::set_uniform<GL_FLOAT_VEC4, vec4 *>
(const char * name, vec4 * value ) {
    glUniform4fv ( glGetUniformLocation(_program, name), 1, (float * )value);
}

// ------------
template<>
void ShaderProgram::set_uniform<GL_FLOAT_MAT2, float *>
(const char * name, float * value ) {
    glUniformMatrix3fv ( glGetUniformLocation(_program, name), 1, GL_FALSE,
                        (float * )value);
}

template<>
void ShaderProgram::set_uniform<GL_FLOAT_MAT2, mat2 *>
(const char * name, mat2 * value ) {
    glUniformMatrix3fv ( glGetUniformLocation(_program, name), 1, GL_FALSE,
                        (float * )value);
}

// ------------
template<>
void ShaderProgram::set_uniform<GL_FLOAT_MAT3, float *>
(const char * name, float * value ) {
    glUniformMatrix3fv ( glGetUniformLocation(_program, name), 1, GL_FALSE,
                        (float * )value);
}

template<>
void ShaderProgram::set_uniform<GL_FLOAT_MAT3, mat3 *>
(const char * name, mat3 * value ) {
    glUniformMatrix3fv ( glGetUniformLocation(_program, name), 1, GL_FALSE,
                        (float * )value);
}

// ------------
template<>
void ShaderProgram::set_uniform<GL_FLOAT_MAT4, float *>
(const char * name, float * value ) {
    glUniformMatrix4fv ( glGetUniformLocation(_program, name), 1, GL_FALSE,
                        (float * )value);
}
template<>
void ShaderProgram::set_uniform<GL_FLOAT_MAT4, mat4 *>
(const char * name, mat4 * value ) {
    glUniformMatrix4fv ( glGetUniformLocation(_program, name), 1, GL_FALSE,
                        (float * )value);
}

 void ShaderProgram::set_int  (const char* name, s32 v) {
    set_uniform<GL_INT, s32>(name,v); }

 void ShaderProgram::set_float(const char* name, float v) {
    set_uniform<GL_FLOAT, float>(name,v); }

 void ShaderProgram::set_vec2(const char* name, vec2* v) {
    set_uniform<GL_FLOAT_VEC2, vec2*>(name,v); }

 void ShaderProgram::set_vec3(const char* name, vec3* v) {
    set_uniform<GL_FLOAT_VEC3, vec3*>(name,v); }

 void ShaderProgram::set_mat3(const char* name, mat3* v) {
    set_uniform<GL_FLOAT_MAT3, mat3*>(name,v); }

 void ShaderProgram::set_mat4(const char* name, mat4* v) {
    set_uniform<GL_FLOAT_MAT4, mat4*>(name,v); }

 void ShaderProgram::set_tex2D(const char* name, s32 v) {
    set_uniform<GL_SAMPLER_2D, s32>(name,v); }

 void ShaderProgram::set_tex2D(const char* name, AssetHandle v) {
    set_uniform<GL_SAMPLER_2D, AssetHandle>(name,v); }

 ShaderProgram& ShaderProgram::$_set_int(const char* name, s32 v) {
    set_uniform<GL_INT, s32>(name,v); return *this; }

 ShaderProgram& ShaderProgram::$_set_float(const char* name, float v) {
    set_uniform<GL_FLOAT, float>(name,v); return *this; }

 ShaderProgram& ShaderProgram::$_set_vec2(const char* name, vec2* v) {
    set_uniform<GL_FLOAT_VEC2, vec2*>(name,v); return *this; }

 ShaderProgram& ShaderProgram::$_set_vec3(const char* name, vec3* v) {
    set_uniform<GL_FLOAT_VEC3, vec3*>(name,v); return *this; }

 ShaderProgram& ShaderProgram::$_set_mat3(const char* name, mat3* v) {
    set_uniform<GL_FLOAT_MAT3, mat3*>(name,v); return *this; }

 ShaderProgram& ShaderProgram::$_set_mat4(const char* name, mat4* v) {
    set_uniform<GL_FLOAT_MAT4, mat4*>(name,v); return *this; }

 ShaderProgram& ShaderProgram::$_set_tex2D(const char* name, s32 v) {
    set_uniform<GL_SAMPLER_2D, s32>(name,v); return *this; }

 ShaderProgram& ShaderProgram::$_set_tex2D(const char* name, AssetHandle v) {
    set_uniform<GL_SAMPLER_2D, AssetHandle>(name,v); return *this; }


void ShaderProgram::set_material(Material * material) {
    assert(material);
    assert(proto::context);
    auto& ctx = *proto::context;

    set_uniform <GL_FLOAT_VEC3> ("u_material.ambient",
                                 &material->ambient_color);

    set_uniform <GL_FLOAT_VEC3> ("u_material.diffuse",
                                 &material->diffuse_color);

    set_uniform <GL_FLOAT_VEC3> ("u_material.specular",
                                 &material->specular_color);

    set_uniform <GL_FLOAT> ("u_material.shininess",
                            material->shininess);

    Texture2D * map;

    if( (map = get_asset<Texture2D>(material->ambient_map)) )
        set_uniform<GL_SAMPLER_2D>
            ("u_material.ambient_map", gfx::bind_texture(map));
    else
        set_uniform<GL_SAMPLER_2D>
            ("u_material.ambient_map",
             gfx::bind_texture(ctx.default_black_texture_h));

    if( (map = get_asset<Texture2D>(material->diffuse_map)) )
        set_uniform<GL_SAMPLER_2D>
            ("u_material.diffuse_map", gfx::bind_texture(map));
    else
        set_uniform<GL_SAMPLER_2D>
            ("u_material.diffuse_map",
             gfx::bind_texture(ctx.default_white_texture_h));

    if( (map = get_asset<Texture2D>(material->specular_map)) )
        set_uniform<GL_SAMPLER_2D>
            ("u_material.specular_map", gfx::bind_texture(map));
    else
        set_uniform<GL_SAMPLER_2D>
            ("u_material.specular_map",
             gfx::bind_texture(ctx.default_black_texture_h));

    if( (map = get_asset<Texture2D>(material->bump_map)) )
        set_uniform<GL_SAMPLER_2D>
            ("u_material.bump_map", gfx::bind_texture(map));
    else
        set_uniform<GL_SAMPLER_2D>
            ("u_material.bump_map",
             gfx::bind_texture(ctx.default_black_texture_h));

    if( (map = get_asset<Texture2D>(material->opacity_map)) )
        set_uniform<GL_SAMPLER_2D>
            ("u_material.opacity_map", gfx::bind_texture(map));
    else
        set_uniform<GL_SAMPLER_2D>
            ("u_material.opacity_map",
             gfx::bind_texture(ctx.default_white_texture_h));

}



} // namespace proto
