#pragma once

#include "proto/core/platform/macros.hh"

#if defined(PROTO_PLATFORM_WINDOWS)
# include "proto/core/platform/windows-context.hh"
# include "proto/core/platform/windows-clock.hh"
#else
# include "proto/core/platform/linux-context.hh"
# include "proto/core/platform/linux-clock.hh"
#endif

#include "proto/core/graphics/Mesh.hh"
#include "proto/core/graphics/Texture.hh"
#include "proto/core/graphics/Material.hh"
#include "proto/core/graphics/ShaderProgram.hh"
#include "proto/core/memory/LinkedListAllocator.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/containers/StringArena.hh"
#include "proto/core/event-system.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/asset-system/AssetRegistry.hh"
#include "proto/core/input.hh"
#include "proto/core/util/Bitfield.hh"
#include "proto/core/util/ModCounter.hh"

namespace proto {
    // context is all global state of the engine
    // NOTE(kacper):
    // since it inherits platform specific variables it perhaps should
    // be under 'platform' namespace but it is meant to be used too often
    // and those variables are not supposed to be touched by the client anyway

    // it is split into few contexts that are inherited by one final one
    // this is because there are some cases when its handy to have
    // temporary context or oftentimes only a specific part of it

#if defined(PROTO_OPENGL)
    //NOTE(kacper): Context ofc in proto, not OpenGL, sense of the word,
    //              (though in both cases they mean more or less the same thing)
    //              it therefore is not supposed to reflect OpenGL context but
    //              rather just store whatever OpenGL-specific data we need to.
    struct OpenGLContext {
        struct TextureSlot {
            AssetHandle texture = invalid_asset_handle;
            s32 gl_tex_unit;
            s32 gl_id_bound;
            Bitfield<u8> flags;
            constexpr static u8 bound_bit = BIT(0);
            constexpr static u8 fresh_bit = BIT(1);
        };
        Array<TextureSlot> texture_slots;
        // temp
        ModCounter<u32> texture_slots_index;


        graphics::ShaderProgram * current_shader = nullptr; //tmp
        AssetHandle default_ambient_map;
        AssetHandle default_diffuse_map;
        AssetHandle default_specular_map;
        AssetHandle default_bump_map;
    };
#endif

    struct AssetContext {
        AssetRegistry assets;
        memory::LinkedListAllocator asset_metadata_allocator;
        memory::LinkedListAllocator gp_texture_allocator;

        Array<Mesh> meshes;
        Array<Material> materials;
        Array<Texture> textures;

        StringArena asset_paths;
    };

    struct Context :
        AssetContext,

#   if defined(PROTO_OPENGL)
        OpenGLContext,
#   endif

#   if defined(PROTO_PLATFORM_WINDOWS)
        platform::WindowsContext
#   else
        platform::LinuxContext
#   endif
    {
        memory::LinkedListAllocator memory;
        memory::LinkedListAllocator gp_file_buffering_allocator;
        memory::LinkedListAllocator gp_string_allocator;
        memory::LinkedListAllocator gp_debug_strings_allocator;
        StringArena cmdline;

        int count = 0;
        proto::ivec2 window_size;

        StringView clientlib_path;
        char * cwd_path = nullptr;
        char * exe_path = nullptr;

        Channel<KeyEvent> key_input_channel;
        Channel<MouseEvent> mouse_input_channel;

        bool exit_sig = false;
#    if defined(PROTO_PLATFORM_WINDOWS)
        platform::WindowsClockImpl clock;
#    else
        platform::LinuxClockImpl clock;
#    endif
 
    };
}
