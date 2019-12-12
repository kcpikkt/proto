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
#include "proto/core/graphics/Texture2D.hh"
#include "proto/core/graphics/Material.hh"
#include "proto/core/graphics/Cubemap.hh"
#include "proto/core/graphics/ShaderProgram.hh"
#include "proto/core/memory/LinkedListAllocator.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/containers/StringArena.hh"
#include "proto/core/event-system.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/asset-system/AssetRegistry.hh"

#include "proto/core/graphics/Camera.hh"
#include "proto/core/graphics/Framebuffer.hh"

#include "proto/core/entity-system/common.hh"

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

    struct RenderContext {
        struct TextureSlot {
            AssetHandle texture = invalid_asset_handle;
            s32 gl_tex_unit;
            s32 gl_id_bound;
            Bitfield<u8> flags;
            AssetTypeIndex type;
            constexpr static u8 bound_bit = BIT(0);
            constexpr static u8 fresh_bit = BIT(1);
            constexpr static u8 reserved_bit = BIT(2);
        };
        Array<TextureSlot> texture_slots;
        // temp
        ModCounter<u32> texture_slots_index;

        ShaderProgram * current_shader = nullptr; //tmp

        Array<Framebuffer> framebuffers;
        Framebuffer * current_read_framebuffer = nullptr;
        Framebuffer * current_draw_framebuffer = nullptr;

        Framebuffer * default_framebuffer = nullptr;

        AssetHandle default_black_texture_h;
        AssetHandle default_white_texture_h;
        AssetHandle default_checkerboard_texture_h;

        AssetHandle quad_h;
        AssetHandle quad_shader_h;

        AssetHandle gbuffer_shader_h;
        //tmp
        u32 gbuf_FBO;
        u32 gbuf_position_tex;
        u32 gbuf_normal_tex;
        u32 gbuf_albedo_spec_tex;

        Camera camera;
    };

    struct EntityContext {
        struct {
            EntityId _id = 0;
        } entity_generator_data;

        Array<Entity> entities;

        // TODO(kacper): get yourself some hashmaps
        struct {
            Array<RenderMeshComp> render_mesh;
            Array<TransformComp> transform;
        } comp;
    };

    struct AssetContext {
        AssetRegistry assets;
        memory::LinkedListAllocator asset_metadata_allocator;
        memory::LinkedListAllocator gp_texture_allocator;

        Array<Mesh> meshes;
        Array<Material> materials;
        Array<Texture2D> textures;
        Array<Cubemap> cubemaps;
        Array<ShaderProgram> shader_programs;

        StringArena asset_paths;
        StringArena shader_paths;
    };

    struct Context :
        AssetContext,
        EntityContext,
        RenderContext,

#   if defined(PROTO_PLATFORM_WINDOWS)
        platform::WindowsContext
#   else
        platform::LinuxContext
#   endif
    {
        int test = 12;
        memory::LinkedListAllocator memory;
        memory::LinkedListAllocator gp_file_buffering_allocator;
        memory::LinkedListAllocator gp_string_allocator;
        memory::LinkedListAllocator gp_debug_strings_allocator;
        Array<StringView> cmdline;

        proto::ivec2 window_size;

        StringView clientlib_path;
        char * cwd_path = nullptr;
        char * exe_path = nullptr;

        Channel<KeyEvent> key_input_channel;
        Channel<MouseMoveEvent> mouse_move_input_channel;
        Channel<MouseButtonEvent> mouse_button_input_channel;

        Bitfield<debug::Category> stdout_log_categories = debug::category::all;
        debug::Level stdout_log_level = debug::level::all;

        bool exit_sig = false;
#    if defined(PROTO_PLATFORM_WINDOWS)
        platform::WindowsClockImpl clock;
#    else
        platform::LinuxClockImpl clock;
#    endif
 
    };
}
