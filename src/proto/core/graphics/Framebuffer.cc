#include "proto/core/graphics/Framebuffer.hh"
#include "proto/core/graphics/gl.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/context.hh"

namespace proto {

    void Framebuffer::init(ivec2 size, u32 init_color_attachments_count) {
        this->size = size;
        color_attachments.init(init_color_attachments_count, &context->memory);
        glGenFramebuffers(1, &FBO);
    }

    u32 Framebuffer::add_color_attachment(Texture2D * tex) {
        if(size != tex->size)
            debug_warn(debug::category::graphics,
                       "Attaching texture of size ", tex->size,
                       " to Framebufffer of size ", size);

        color_attachments.push_back(tex->handle);
        //TODO(kacper): test if you have to upload it here or just bind
        graphics::gpu_upload(tex);

        auto n = color_attachments.size() - 1;
        glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n,
                                GL_TEXTURE_2D, tex->gl_id, 0);

        return n;
    }
    static u32 attachments[10] = 
        {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
         GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
         GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7, GL_COLOR_ATTACHMENT8,
         GL_COLOR_ATTACHMENT9
        };

    void Framebuffer::finalize() {
        assert(color_attachments.size() <= 10);
        glDrawBuffers(color_attachments.size(), attachments);
    }

} // namespace proto

