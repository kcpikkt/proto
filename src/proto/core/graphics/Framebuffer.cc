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

    u32 Framebuffer::add_color_attachment(Texture2D& texture) {
        if(size != texture.size)
            debug_warn(debug::category::graphics,
                       "Attaching texture of size ", texture.size,
                       " to framebufffer of size ", size);

        color_attachments.push_back(texture.handle);
        if(texture.flags.check(TextureInterface::on_gpu_bit))
            graphics::bind_texture(&texture);
        else
            graphics::gpu_upload(&texture);

        auto n = color_attachments.size() - 1;
        glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + n,
                                GL_TEXTURE_2D, texture.gl_id, 0);

        assert(!glGetError());
        return n;
    }

    void Framebuffer::add_depth_attachment(Texture2D & texture) {
        if(size != texture.size)
            debug_warn(debug::category::graphics,
                       "Attaching texture of size ", texture.size,
                       " to framebuffer of size ", size);

        if(texture.flags.check(TextureInterface::on_gpu_bit))
            graphics::bind_texture(&texture);
        else
            graphics::gpu_upload(&texture);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                               GL_TEXTURE_2D, texture.gl_id, 0);

    }

    void Framebuffer::add_depth_attachment(Renderbuffer & renderbuffer) {
       if(size != renderbuffer.size)
            debug_warn(debug::category::graphics,
                       "Attaching renderbuffer of size ", renderbuffer.size,
                       " to framebuffer of size ", size);

        graphics::gpu_upload(&renderbuffer);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, renderbuffer.gl_id);
    }

    static u32 attachments[10] = 
        {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
         GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5,
         GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7, GL_COLOR_ATTACHMENT8,
         GL_COLOR_ATTACHMENT9
        };

    void Framebuffer::finalize() {
        assert(color_attachments.size() <= 10);
        if(color_attachments.size()) {
            glDrawBuffers(color_attachments.size(), attachments);
        } else {
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);
        }

        u32 errcode = glCheckFramebufferStatus(GL_FRAMEBUFFER);

        if(errcode != GL_FRAMEBUFFER_COMPLETE) {
            const char * errmsg = "(reason unknown)";

            switch(errcode) {
            case GL_FRAMEBUFFER_UNDEFINED:
                errmsg = "specified framebuffer is the default read or draw "
                    " framebuffer, but the default framebuffer does not exist.";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                errmsg =
                    "one or more framebuffer attachments "
                    "are framebuffer incomplete.";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                errmsg = "is returned if the framebuffer does not have at least "
                    "one image attached to it.";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
                errmsg = "is returned if the value of "
                    "GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE for any "
                    "color attachment point(s) named by GL_DRAW_BUFFERi.";

            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
                errmsg = "is returned if GL_READ_BUFFER is not GL_NONE and "
                    "the value of GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE is GL_NONE "
                    "for the color attachment point named by GL_READ_BUFFER.";
                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                errmsg = "the combination of internal formats of the attached "
                    "images violates an implementation-dependent "
                    "set of restrictions.";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
                errmsg = "the value of GL_RENDERBUFFER_SAMPLES is not the same "
                    "for all attached renderbuffers; or the value of "
                    "GL_TEXTURE_SAMPLES is the not same for all attached textures; "
                    "or, if the attached images are a mix of renderbuffers and "
                     "textures, the value of GL_RENDERBUFFER_SAMPLES."
                    "It is also possible that does not match the value of "
                    "GL_TEXTURE_SAMPLES or the value of "
                    "GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not "
                    "the same for all attached textures; or, if the attached "
                    "images are a mix of renderbuffers and textures, the value of "
                    "GL_TEXTURE_FIXED_SAMPLE_LOCATIONS is not GL_TRUE "
                    "for all attached textures.";
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
                errmsg = "is returned if any framebuffer attachment is layered, "
                    "and any populated attachment is not layered, "
                    "or if all populated color attachments "
                    "are not from textures of the same target.";
                break;
            }
            debug_error(debug::category::graphics,
                        "Incomplete framebuffer: ", errmsg);
        }
    }

    // chaining methods
    Framebuffer& Framebuffer::$_init(ivec2 size, u32 init_color_attachments_count) {
        init(size, init_color_attachments_count); return *this;
    }

    Framebuffer& Framebuffer::$_bind() {
        graphics::bind_framebuffer(*this); return *this;
    }

    Framebuffer& Framebuffer::$_add_color_attachment(Texture2D & texture) {
        add_color_attachment(texture); return *this;
    }

    Framebuffer& Framebuffer::$_add_depth_attachment(Renderbuffer & renderbuffer) {
        add_depth_attachment(renderbuffer); return *this;
    }

    Framebuffer& Framebuffer::$_add_depth_attachment(Texture2D & texture) {
        add_depth_attachment(texture); return *this;
    }
} // namespace proto

