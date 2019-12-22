#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/graphics/Texture2D.hh"
#include "proto/core/graphics/Renderbuffer.hh"
namespace proto {

struct Framebuffer {
    u32 FBO;
    ivec2 size;
    Array<AssetHandle> color_attachments;

    void init(ivec2 size, u32 init_color_attachments_count = 1);

    u32 add_color_attachment(Texture2D&); // overload for renderbuffer
    void add_depth_attachment(Renderbuffer&);
    void add_depth_attachment(Texture2D&);
    void add_depth_attachment(Cubemap&); 

    // chaining methods
    Framebuffer& $_init(ivec2 size, u32 init_color_attachments_count);
    Framebuffer& $_bind();
    Framebuffer& $_add_color_attachment(Texture2D&);
    Framebuffer& $_add_depth_attachment(Renderbuffer&);
    Framebuffer& $_add_depth_attachment(Texture2D&);
    Framebuffer& $_add_depth_attachment(Cubemap&);

    void finalize();
};

}
