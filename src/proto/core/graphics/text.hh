#pragma once
#include "proto/core/common.hh"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace proto {
    struct GlyphSSBOStruct {
        proto::vec2 pos;
        proto::vec2 size;
        uint32_t ch;
        uint32_t pad;
    };

    struct ASCIIFontAtlas {
        struct GlyphDesc {
            proto::ivec2 coords;
            proto::ivec2 size;
            proto::ivec2 bearing;
            int64_t advance;
        };
        struct GPUGlyphDesc {
            proto::ivec2 coords;
            proto::ivec2 size;
        };
        Texture texture;
        GlyphDesc glyph_desc[128];
        GPUGlyphDesc gpu_glyph_desc[128];
    };


    void create_ascii_font_atlas(ASCIIFontAtlas * ret, int8_t font_size) {
        FT_Library ft;
        if(FT_Init_FreeType(&ft))
            puts("failed to init freetype");
        FT_Face face;
        if(FT_New_Face(ft, "arial.ttf", 0, &face))
            debug_warn(proto::debug::category::graphics,
                       "failed to load font");

        FT_Set_Pixel_Sizes(face, 0, font_size);

        if(FT_Load_Char(face, 'X', FT_LOAD_RENDER))
            debug_warn(proto::debug::category::graphics,
                       "failed to load test glyph");

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glm::ivec2 offset = glm::ivec2(0, 0);
        glm::ivec2 atlas_size(16 * font_size, 8 * font_size);
        ret->texture = Texture(atlas_size);

        for(u8 c=0; c<128; c++) {
            if(FT_Load_Char(face, c, FT_LOAD_RENDER)){
                printf("failed to load character %c\n", (char)c);
                continue;
            }

            if(offset.x + face->glyph->bitmap.width > atlas_size.x) {
                offset.x = 0; offset.y += font_size;
            }

            ret->glyph_desc[c] = ASCIIFontAtlas::GlyphDesc {
                .coords = offset,
                .size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                .bearing = glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                .advance = face->glyph->advance.x
            };

            ret->gpu_glyph_desc[c] = ASCIIFontAtlas::GPUGlyphDesc {
                .coords = offset,
                .size = glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            };

            auto& desc = ret->glyph_desc[c];

            glBindTexture(GL_TEXTURE_2D, ret->texture.id);

            glTexSubImage2D(GL_TEXTURE_2D, 0, offset.x, offset.y,
                            desc.size.x, desc.size.y, GL_RED, GL_UNSIGNED_BYTE,
                            face->glyph->bitmap.buffer);

            offset.x += desc.size.x;
        }
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
    }

    template<typename T, typename C>
    struct CRTP {
        using component_t = C;
        using allocator_t = matter::memory::linear_allocator;
        inline static allocator_t _allocator {};
        inline static matter::dynamic_array<Entity, allocator_t> keys {};
        inline static matter::dynamic_array<component_t, allocator_t> components {};

        static component_t & create_component(Entity& entity, component_t comp) {
               return T::create_component(entity, comp);
        //     keys.push_back(entity);
        //     comp.entity = entity;
        //     components.push_back(comp);
        //     return components.back();
        }
    };

    struct Something {};
    struct BatchOfSomething : CRTP<BatchOfSomething, Something> {};
    struct TextRect_Comp : Component{
        Transform_Comp transform;
        const char * text;
        glm::vec2 rect;
    };
    template<> struct Batch<TextRect_Comp> : BasicBatch<TextRect_Comp>{
        inline static matter::Shader text_shader;

        inline static GLuint glyph_quad_VAO, glyph_quad_VBO;

        inline constexpr static size_t gpu_batch_size = 256;
        inline static GLuint gpu_glyphs_ssbo {};
        inline static GlyphSSBOStruct gpu_glyphs[gpu_batch_size] = {};
        inline static ASCIIFontAtlas test_font_atlas{};

        //FIXME:
        inline static memory::linear_allocator string_arena;

        static int init() {
            _allocator.init(matter::memory::megabytes(2));

            keys.init(256, &_allocator);
            components.init(256, &_allocator);
            string_arena.init(memory::megabytes(2));

            const char * text_vert_shader = R"glsl(
                #version 430 core
                layout (location = 0) in vec2 quad;

                out vec2 p_quad;
                flat out int instance_id;
                uniform ivec4 u_glyph_desc[128];
                uniform float scale;

                struct GlyphSSBOStruct {
                    vec2 pos;
                    vec2 size;
                    uint ch;
                    uint pad;
                };
                layout(std430, binding = 0) buffer glyphbuf {
                    GlyphSSBOStruct glyphs[];
                };

                void main() {
                    p_quad = quad;

                    instance_id = gl_InstanceID;

                    GlyphSSBOStruct glyph = glyphs[gl_InstanceID];
                    vec2 glyph_screen_pos = glyph.pos;

                    gl_Position = vec4(glyph_screen_pos + quad * glyph.size * scale
                                        , 0.0, 1.0);
                }
            )glsl";

            const char * text_frag_shader = R"glsl(
                #version 430 core

                in vec2 p_quad;
                    flat in int instance_id;
                out vec4 color;

                uniform sampler2D tex;
                uniform vec3 text_color;
                uniform ivec4 u_glyph_desc[128];
                uniform float scale;

                struct GlyphSSBOStruct {
                    vec2 pos;
                    vec2 size;
                    uint ch;
                    uint pad;
                };
                layout(std430, binding = 0) buffer glyphbuf {
                    GlyphSSBOStruct glyphs[];
                };

                void main() {
                    GlyphSSBOStruct glyph = glyphs[instance_id];
                    vec2 glyph_screen_pos = glyph.pos ;

                    ivec4 glyph_desc = u_glyph_desc[glyph.ch];

                    ivec2 tex_size = textureSize(tex, 0);
                    vec2 glyph_tex_offset = (vec2(glyph_desc.xy) / vec2(tex_size));
                    vec2 glyph_tex_size = (vec2(glyph_desc.zw) / vec2(tex_size));

                    //TODO: make uv and world coodinate system the same orientation
                    vec2 uv = glyph_tex_offset +
                            abs(p_quad - vec2(0.0,1.0)) * glyph_tex_size;
                    color = vec4(text_color, texture(tex, uv).r);
                }
            )glsl";

            text_shader = matter::Shader(text_vert_shader, text_frag_shader);


            float quad[8] = { 0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0,};

            glyph_quad_VAO = glyph_quad_VBO = 0;
            glGenVertexArrays(1, &glyph_quad_VAO);
            glGenBuffers(1, &glyph_quad_VBO);
            glBindVertexArray(glyph_quad_VAO);
            glBindBuffer(GL_ARRAY_BUFFER, glyph_quad_VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8, quad, GL_STREAM_DRAW);
            glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            create_ascii_font_atlas(&test_font_atlas, 42);

            glGenBuffers(1, &gpu_glyphs_ssbo);
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu_glyphs_ssbo);
            glBufferData(GL_SHADER_STORAGE_BUFFER,
                         sizeof(GlyphSSBOStruct) * gpu_batch_size,
                         NULL, GL_DYNAMIC_DRAW);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gpu_glyphs_ssbo);

            return 0;
        }

        static void assemble_gpu_data(float scale) {
            glm::vec2 _pos;
            glm::vec2 pos = glm::vec2(0.0);

            const char * c = "sample text";
            int counter = 0;
            while(*c != '\0' && counter < 100) {
                counter++;
                assert(*c >= 0 && *c < 128);
                auto& ch = test_font_atlas.glyph_desc[*c];

                _pos.x = pos.x + ch.bearing.x * scale;
                _pos.y = pos.y - (ch.size.y - ch.bearing.y)* scale;

                gpu_glyphs[counter].pos  = _pos;
                gpu_glyphs[counter].size = ch.size;
                gpu_glyphs[counter].ch   = *c;
                pos.x += (ch.advance >> 6) * scale;
                c++;
            }
        }

        static void draw(float scale) {
            text_shader.use();
            glm::vec3 u_text_color = glm::vec3(1.0f,1.0f,1.0f);
            text_shader.set_uniform("text_color", GL_FLOAT_VEC3, &u_text_color[0]);
            text_shader.set_uniform("scale", GL_FLOAT, scale);
            glm::vec2 u_resolution = glm::vec2(1024, 1024);
            text_shader.set_uniform("u_resolution", GL_FLOAT_VEC2, &u_resolution[0]);
            glUniform4iv(glGetUniformLocation(text_shader.program, "u_glyph_desc"),
                         128, (int*)& (test_font_atlas.gpu_glyph_desc[0]));


            glBindBuffer(GL_SHADER_STORAGE_BUFFER, gpu_glyphs_ssbo);
            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0 ,
                            sizeof(GlyphSSBOStruct) * gpu_batch_size,
                            gpu_glyphs);

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, gpu_glyphs_ssbo);

            glBindVertexArray(glyph_quad_VAO);
            glBindBuffer(GL_ARRAY_BUFFER, glyph_quad_VBO);
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, 10);
        }

        static void process() {
            assemble_gpu_data(2.0f/(1024.0f));
            draw(2.0f/(1024.0f));
            // for(size_t i=0; i<components.size(); i++) {
            //     auto& comp = components[i];
            // }
        }
        static void sync() {
            for(size_t i=0; i<components.size(); i++) {
                components[i].transform =
                    components[i].entity.get_component<Transform_Comp>().value;
            }
        }
    };
    /*******************************************************************
TEXT RENDERING END HERRE
     */
} // namespace proto 

