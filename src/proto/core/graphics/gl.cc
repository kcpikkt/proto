#include "proto/core/graphics/gl.hh"
#include "proto/core/graphics/common.hh"
#include "proto/core/context.hh"
#include "proto/core/graphics/Texture2D.hh"
#include "proto/core/graphics/Cubemap.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/util/algo.hh"
#include "proto/core/graphics/Vertex.hh"

namespace proto {
namespace graphics{
    //namespace gl{

    s32 bind_texture(AssetHandle texture_handle) {
        auto fail =
            [](){
                debug_warn(debug::category::graphics,
                           "Texture handle passed to ", __PRETTY_FUNCTION__,
                           " is invalid, no bindig performed");
                return -1;};
        auto type = AssetType(texture_handle.type).index;

        if(!texture_handle) return fail();

        /**/ if(type == AssetType<Texture2D>::index)
            return bind_texture(get_asset<Texture2D>(texture_handle));
        else if(type == AssetType<Cubemap>::index)
            return bind_texture(get_asset<Cubemap>(texture_handle));
        else
            return fail();
    }

    template<typename T>
    auto bind_texture(T * texture) ->
        meta::enable_if_t<meta::is_base_of_v<TextureInterface, T>,s32>
    {
        if(!texture) {
            debug_warn(debug::category::graphics,
                       "Pointer to texture passed to ", __PRETTY_FUNCTION__,
                       " is null, no bindig performed");
            return -1;
        }
        return bind_texture(*texture);
    }

    template<typename T>
    auto bind_texture(T & texture) ->
        meta::enable_if_t<meta::is_base_of_v<TextureInterface, T>,s32>
    {
        assert(proto::context);
        auto& ctx = *proto::context;

        using Slot = RenderContext::TextureSlot;
        
        if(texture.flags.check(TextureInterface::bound_bit)) {
            assert(belongs(texture.bound_unit, 0, (s32)ctx.texture_slots.size()));
            auto& slot = ctx.texture_slots[texture.bound_unit];

            if(!slot.flags.check(Slot::reserved_bit)) {
                assert(slot.flags.check(Slot::bound_bit));
                assert(slot.texture == texture.handle);
                assert(slot.bound_gl_id == texture.gl_id);
                slot.flags.set(Slot::fresh_bit);
                return texture.bound_unit;
            }
        }

        auto& modindex = ctx.texture_slots_index;
        u32 loopcount = 0;
        do {
            if(loopcount++ >= ctx.texture_slots.size()) return -1;// no hit
        } while(modindex++,
                ctx.texture_slots[modindex].flags.check(Slot::fresh_bit) ||
                ctx.texture_slots[modindex].flags.check(Slot::reserved_bit) );

        auto& slot = context->texture_slots[modindex];
        AssetHandle prev_handle = slot.texture;

        if(prev_handle) {
            TextureInterface * prev_texture =
                get_asset<TextureInterface>(prev_handle);
            assert(prev_texture);
            if(!(prev_texture->bound_unit == (s32)modindex)) {
                debug_print_texture_slots();
                vardump(prev_texture->bound_unit);
                vardump((u8)modindex);
                vardump(get_metadata(prev_texture->handle)->name);
            }
            //assert(prev_texture->bound_unit == (s32)modindex);

            prev_texture->bound_unit = -1;
            prev_texture->flags.unset(TextureInterface::bound_bit);
        }

        assert(texture.gl_id >= 0);
        glActiveTexture(GL_TEXTURE0 + modindex);

        /*  */ if constexpr(meta::is_same_v<T, Texture2D>) {
            slot.type = AssetType<Texture2D>::index;
            glBindTexture(GL_TEXTURE_2D, texture.gl_id);
        } else if constexpr(meta::is_same_v<T, Cubemap>) {
            slot.type = AssetType<Cubemap>::index;
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture.gl_id);
        }

        slot.texture = texture.handle;

        texture.bound_unit = (s32)modindex;
        slot.bound_gl_id = texture.gl_id;

        texture.flags.set(TextureInterface::bound_bit);
        slot.flags.set(Slot::bound_bit | Slot::fresh_bit);
        // cant bind reserved slot!
        assert(!slot.flags.check(Slot::reserved_bit));

        assert(belongs(texture.bound_unit, 0ul, ctx.texture_slots.size()));

        return modindex;
    }

    ////////////////////////////////////////////////////////////////////

    s32 unbind_texture(AssetHandle texture_handle) {
        assert(!glGetError());
        auto fail =
            [](){
                debug_warn(debug::category::graphics,
                           "Texture handle passed to ", __PRETTY_FUNCTION__,
                           " is invalid, no unbindig performed");
                return -1;};
        auto type = AssetType(texture_handle.type).index;

        if(!texture_handle) return fail();

        /**/ if(type == AssetType<Texture2D>::index)
            return unbind_texture(get_asset<Texture2D>(texture_handle));
        else if(type == AssetType<Cubemap>::index)
            return unbind_texture(get_asset<Cubemap>(texture_handle));
        else
            return fail();
    }

    template<typename T>
    auto unbind_texture(T * texture) ->
        meta::enable_if_t<meta::is_base_of_v<TextureInterface, T>,s32>
    {
        assert(!glGetError());
        if(!texture) {
            debug_warn(debug::category::graphics,
                       "Pointer to texture passed to ", __PRETTY_FUNCTION__,
                       " is null, no bindig performed");
            return -1;
        }
        return unbind_texture(*texture);
    }

    template<typename T>
    auto unbind_texture(T & texture) ->
        meta::enable_if_t<meta::is_base_of_v<TextureInterface, T>,s32>
    {
        assert(proto::context);
        auto& ctx = *proto::context;

        using Slot = RenderContext::TextureSlot;
        s32 slot_index;
        
        if(texture.flags.check(TextureInterface::bound_bit)) {
            assert(belongs(texture.bound_unit, 0, (s32)ctx.texture_slots.size()));

            slot_index = texture.bound_unit;
            auto& slot = ctx.texture_slots[texture.bound_unit];

            assert(slot.flags.check(Slot::bound_bit));
            assert(slot.texture == texture.handle);

            slot.flags.unset(Slot::fresh_bit);
            slot.flags.unset(Slot::bound_bit);
            slot.texture = invalid_asset_handle;

            texture.flags.unset(TextureInterface::bound_bit);
            texture.bound_unit = -1;
        } else {
            auto * metadata = get_metadata(texture.handle);
            assert(metadata);
            debug_warn(debug::category::graphics,
                       "Tried to unbind not bound texture ", metadata->name);
            return -1;
        }
        return slot_index;
    }
 

    template s32 bind_texture<Texture2D>(Texture2D*);
    template s32 bind_texture<Cubemap>(Cubemap*);
    template s32 bind_texture<TextureInterface>(TextureInterface*);

    template s32 bind_texture<Texture2D>(Texture2D&);
    template s32 bind_texture<Cubemap>(Cubemap&);


    template s32 unbind_texture<Texture2D>(Texture2D*);
    template s32 unbind_texture<Cubemap>(Cubemap*);
    template s32 unbind_texture<TextureInterface>(TextureInterface*);

    template s32 unbind_texture<Texture2D>(Texture2D&);
    template s32 unbind_texture<Cubemap>(Cubemap&);


    void unbind_texture_slot(s32 index) {
        assert(proto::context);
        auto& ctx = *proto::context;
        vardump(index);
        assert(belongs(index, 0, (s32)ctx.texture_slots.size()));

        using Slot = RenderContext::TextureSlot;
        auto& slot = ctx.texture_slots[index];

        if(slot.flags.check(Slot::bound_bit)) {
            Texture2D * texture = get_asset<Texture2D>(slot.texture);
            assert(texture);
            assert(texture->flags.check(Texture2D::bound_bit));
            assert(index == texture->bound_unit);

            texture->flags.unset(Texture2D::bound_bit);
            texture->bound_unit = -1;
            slot.flags.unset(Slot::bound_bit);
            slot.flags.unset(Slot::fresh_bit);
            slot.texture = invalid_asset_handle;
        } else {
            assert(!slot.texture);
        }
    }

    void unbind_all_texture_slots() {
        for(u32 i=0; i<context->texture_slots.size(); i++) unbind_texture_slot(i);
    }

    void stale_texture_slot(u32 index) {
        assert(proto::context);
        using Slot = proto::RenderContext::TextureSlot;
        context->texture_slots[index].flags.unset(Slot::fresh_bit);
    }

    void stale_all_texture_slots() {
        assert(proto::context);
        using Slot = proto::RenderContext::TextureSlot;
        for(auto& s : context->texture_slots) s.flags.unset(Slot::fresh_bit);
    }

    u32 bind_framebuffer(Framebuffer& target) {
        glBindFramebuffer(GL_FRAMEBUFFER, target.FBO);
        context->current_read_framebuffer = &target;
        context->current_draw_framebuffer = &target;
        return target.FBO;
    }

    u32 reset_framebuffer() {
        return bind_framebuffer(*context->default_framebuffer);
    }

    void debug_print_texture_slots() {
        printf("Texture units state\n");
        using Slot = RenderContext::TextureSlot;

        for(u32 i=0; i<context->texture_slots.size(); i++) {
            auto& s = context->texture_slots[i];
            printf("[%.2d:%s]", i, AssetType(s.type).name);
            if(s.texture) {
                auto * metadata = get_metadata(s.texture);
                auto * texture = get_asset<TextureInterface>(s.texture);
                printf(" %d %s", texture->bound_unit, metadata->name);
                if(s.flags.check(Slot::fresh_bit)) printf(" (fresh)");
                puts("");
            } else {
                printf(" (unbound)\n");
            }
        }
    }

    // take refs, nofail here, handle taking function can fail
    template<typename T> void gpu_upload(T *);

    template<> void gpu_upload<Renderbuffer>(Renderbuffer * renderbuffer) {
        assert(renderbuffer);
        if(renderbuffer->flags.check(Renderbuffer::on_gpu_bit)) return;

        glBindRenderbuffer(GL_RENDERBUFFER, renderbuffer->gl_id);
        glRenderbufferStorage(GL_RENDERBUFFER, renderbuffer->gpu_format,
                              renderbuffer->size.x, renderbuffer->size.y);
        renderbuffer->flags.set(Renderbuffer::on_gpu_bit);
    }

    template<> void gpu_upload<Cubemap>(Cubemap * cubemap) {
        assert(cubemap);
        if(cubemap->flags.check(TextureInterface::on_gpu_bit)) return;

        bind_texture(cubemap);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        for(s32 i=0; i<6; i++) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                     cubemap->format,
                     cubemap->size.x, cubemap->size.y, 0,
                     cubemap->gpu_format,
                     GL_UNSIGNED_BYTE, //cubemap->datatype,
                     cubemap->data[i]);
        }
        //if(texture->flags.check(TextureInterface::mipmap_bit))
        //    glGenerateMipmap(GL_TEXTURE_2D);

        cubemap->flags.set(TextureInterface::on_gpu_bit);
        stale_texture_slot(cubemap->bound_unit);
    }

    template<> void gpu_upload<Texture2D>(Texture2D * texture) {
        assert(texture);
        if(texture->flags.check(TextureInterface::on_gpu_bit)) return;

        bind_texture(texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // Texture2D::data should be null default

        if(texture->datatype == GL_FLOAT) {
            vardump(get_metadata(texture->handle)->name);
        }

        glTexImage2D(GL_TEXTURE_2D, 0,
                     texture->format,
                     texture->size.x, texture->size.y, 0,
                     texture->gpu_format,
                     texture->datatype,
                     texture->data);

        if(texture->flags.check(TextureInterface::mipmap_bit))
            glGenerateMipmap(GL_TEXTURE_2D);

        texture->flags.set(TextureInterface::on_gpu_bit);
        stale_texture_slot(texture->bound_unit);
    }


    template<> void gpu_upload<Mesh>(Mesh * mesh) {
        assert(mesh);
        if(mesh->flags.check(Mesh::on_gpu_bit)) return;

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
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                            (void*) (offsetof(struct Vertex, position)) );

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                                (void*) (offsetof(struct Vertex, normal)) );

        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct Vertex),
                                (void*) (offsetof(struct Vertex, uv)) );
        mesh->flags.set(Mesh::on_gpu_bit);

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

    //} // namespace gl
} // namespace graphics
} // namespace proto
