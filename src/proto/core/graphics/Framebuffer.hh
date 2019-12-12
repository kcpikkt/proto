#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/graphics/Texture2D.hh"
namespace proto {

struct Framebuffer {
    u32 FBO;
    ivec2 size;
    Array<AssetHandle> color_attachments;

    void init(ivec2 size, u32 init_color_attachments_count = 1);

    Framebuffer& $_add_color_attachment(Texture2D * tex);

    u32 add_color_attachment(Texture2D * tex);

    void finalize();
};

}
