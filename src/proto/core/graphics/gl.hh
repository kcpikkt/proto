#pragma once
#include "proto/core/graphics/common.hh"
#include "proto/core/context.hh"
#include "proto/core/graphics/Texture.hh"
#include "proto/core/debug/logging.hh"

namespace proto {
namespace graphics{
namespace gl{
    const char * error_message();

    void stale_all_texture_slots();
    void stale_texture_slot(u32 index);

    s32 bind_texture(AssetHandle texture_handle);
    s32 bind_texture(Texture * texture);
    s32 bind_texture(Texture & texture);

    void debug_print_texture_slots();

    template<typename T> void gpu_upload(T *);
    template<> void gpu_upload<Texture>(Texture *);

    const char * error_message();
} // namespace gl
} // namespace graphics
} // namespace proto
