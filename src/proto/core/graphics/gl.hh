#pragma once
#include <glm/glm.hpp>
#include "proto/core/graphics/common.hh"
#include "proto/core/context.hh"
#include "proto/core/graphics/Texture.hh"
#include "proto/core/debug/logging.hh"

namespace proto {
namespace graphics{
namespace gl{
    const char * error_message();

    s32 bind_texture(Texture * texture) {

        if(texture->bound_unit >= 0) {
            assert(context->texture_slots[texture->bound_unit].texture ==
                   texture->handle);
            return texture->bound_unit;
        }

        s32 modindex = context->texture_slots_index % context->texture_slots.size();
        auto& slot = context->texture_slots[modindex];
        AssetHandle prev_handle = slot.texture;

        if(prev_handle) {
            Texture * prev_texture = get_asset<Texture>(prev_handle);
            assert(prev_texture);
            assert(prev_texture->bound_unit >= 0);
            prev_texture->bound_unit = -1;
        }
        glActiveTexture(GL_TEXTURE0 + modindex);
        glBindTexture(GL_TEXTURE_2D, texture->gl_id);
        slot.texture = texture->handle;
        texture->bound_unit = modindex;

        context->texture_slots_index++;
        return modindex;
    }

    s32 bind_texture(AssetHandle texture_handle) {
        return bind_texture(get_asset<Texture>(texture_handle));
    }

    void debug_print_texture_slots() {
        printf("Texture units state\n");
        int count = 0;
        for(auto s : context->texture_slots) {
            count++;
            printf("[%.2d]", count);

            if(s.texture) {
                auto * metadata = get_metadata(s.texture);
                auto * texture = get_asset<Texture>(s.texture);
                printf(" %d %s\n", texture->bound_unit, metadata->name);
            } else {
                printf(" (unbound)\n");
            }
        }
    }

    template<typename T> void gpu_upload(T *);

    template<> void gpu_upload<Texture>(Texture * tex) {
        assert(tex);
        if(!tex->data) {
            debug_warn(debug::category::graphics,
                       "Texture GPU upload failed because passed texture "
                       "does not hold any image data.");
            return;
        }

        bind_texture(tex);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        /*  */ if(tex->channels == 1) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, tex->size.x, tex->size.y,
                         0, GL_RED, GL_UNSIGNED_BYTE, tex->data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else if(tex->channels == 3) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, tex->size.x, tex->size.y,
                         0, GL_RGB, GL_UNSIGNED_BYTE, tex->data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else if(tex->channels == 4) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex->size.x, tex->size.y,
                         0, GL_RGBA, GL_UNSIGNED_BYTE, tex->data);
            glGenerateMipmap(GL_TEXTURE_2D);
        } else {
            debug_warn(debug::category::graphics,
                       "No support for textures with ", tex->channels, " channels");
        }

    }

    template<> void gpu_upload<Mesh>(Mesh * mesh) {
        assert(mesh);
        glBindVertexArray(mesh->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);

        glBufferData(GL_ARRAY_BUFFER,
                    sizeof(proto::Vertex) * mesh->vertices.size(),
                    mesh->vertices.raw(),
                    GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                    sizeof(u32) * mesh->indices.size(),
                    mesh->indices.raw(),
                    GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(proto::Vertex),
                            (void*) (offsetof(proto::Vertex, position)) );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(proto::Vertex),
                                (void*) (offsetof(proto::Vertex, normal)) );

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(proto::Vertex),
                                (void*) (offsetof(proto::Vertex, uv)) );

    }

    const char * error_message() {
        char const * message;
        switch(glGetError()) {
        case GL_NO_ERROR:
            return "(no error)";
        case GL_INVALID_ENUM:
            message = "An unacceptable value is specified for an enumerated argument. The offending command is ignored and has no other side effect than to set the error flag.";
            break;
        case GL_INVALID_VALUE:
            message = "A numeric argument is out of range. The offending command is ignored and has no other side effect than to set the error flag.";
            break;
        case GL_INVALID_OPERATION:
            message = "The specified operation is not allowed in the current state. The offending command is ignored and has no other side effect than to set the error flag.";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            message = "The framebuffer object is not complete. The offending command is ignored and has no other side effect than to set the error flag.";
            break;
        case GL_OUT_OF_MEMORY:
            message = "There is not enough memory left to execute the command. The state of the GL is undefined, except for the state of the error flags, after this error is recorded.";

        case GL_STACK_UNDERFLOW:
            message = "An attempt has been made to perform an operation that would cause an internal stack to underflow.";
            break;
        case GL_STACK_OVERFLOW:
            message = "An attempt has been made to perform an operation that would cause an internal stack to overflow. ";
            break;
        }
        return message;
    }
} // namespace gl
} // namespace graphics
} // namespace proto
