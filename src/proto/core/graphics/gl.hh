#pragma once
#include "proto/core/graphics/common.hh"
#include "proto/core/context.hh"
#include "proto/core/graphics/Texture2D.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/meta.hh"

namespace proto {
namespace graphics{
namespace gl{
    const char * error_message();

    void stale_all_texture_slots();
    void stale_texture_slot(u32 index);

    // lazy option
    s32 bind_texture(AssetHandle texture_handle);

    // for uncertain 
    template<typename T>
    auto bind_texture(T * texture) ->
        meta::enable_if_t<meta::is_base_of_v<TextureInterface, T>,s32>;

    // no exceptions here so do not pass anything null unchecked
    template<typename T>
    auto bind_texture(T & texture) ->
        meta::enable_if_t<meta::is_base_of_v<TextureInterface, T>,s32>;

    s32 unbind_texture(AssetHandle texture_handle);

    template<typename T>
    auto unbind_texture(T * texture) ->
        meta::enable_if_t<meta::is_base_of_v<TextureInterface, T>,s32>;

    template<typename T>
    auto unbind_texture(T & texture) ->
        meta::enable_if_t<meta::is_base_of_v<TextureInterface, T>,s32>;


    void debug_print_texture_slots();

    template<typename T> void gpu_upload(T *);
    template<> void gpu_upload<Texture2D>(Texture2D *);

    const char * error_message();
} // namespace gl
} // namespace graphics
} // namespace proto
