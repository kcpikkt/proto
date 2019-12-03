#pragma once
#include "proto/core/graphics/common.hh"
#include "proto/core/context.hh"
#include "proto/core/graphics/Texture.hh"
#include "proto/core/debug/logging.hh"

namespace proto {
namespace graphics{
namespace gl{
    const char * error_message();
    s32 bind_texture(Texture * texture);

    void free_texture_slots();

    s32 bind_texture(AssetHandle texture_handle);

    void debug_print_texture_slots();

    template<typename T> void gpu_upload(T *);

    const char * error_message();
} // namespace gl
} // namespace graphics
} // namespace proto
