#pragma once

#include "proto/core/platform/macros.hh"

#include "proto/core/platform/PlatformContext.hh"

#include "proto/core/platform/Clock.hh"
#include "proto/core/graphics/Mesh.hh"
#include "proto/core/graphics/Texture2D.hh"
#include "proto/core/graphics/Material.hh"
#include "proto/core/graphics/Cubemap.hh"
#include "proto/core/graphics/ShaderProgram.hh"
#include "proto/core/memory/LinkedListAllocator.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/containers/ArrayMap.hh"
#include "proto/core/containers/StringArena.hh"
#include "proto/core/event-system.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/serialization/Archive.hh"

#include "proto/core/graphics/Camera.hh"
#include "proto/core/graphics/Framebuffer.hh"

#include "proto/core/entity-system/common.hh"

#include "proto/core/input.hh"
#include "proto/core/util/Bitfield.hh"
#include "proto/core/util/ModCounter.hh"
#include "proto/core/util/Pair.hh"

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
            s32 bound_gl_id;
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
        AssetHandle cube_h;

        AssetHandle quad_shader_h;
        AssetHandle skybox_shader_h;
        AssetHandle std_basis_h;
        AssetHandle std_basis_shader_h;

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

        Array<Entity> ents;
        ArrayMap<Entity, EntityMetadata> ents_mdata;

        // This array store pointers to Arrays of components cast to void*, dodgy but 
        // We cannot be more explicit since we avoid runtime polymorphism and any user code as well as
        // majority of proto code don't interact with it directly.
        // When it comes to speed, indices of arrays of default components are resolved statically anyway,
        // however this allows client to define their own game specific components, and this is crutial.
        // TODO(kacper): it may be just elegant to use some kind of owning pointer when it comes to those arrays
        //               but it is not vital atm - components shall not hold any data/state to cleanup
        //               and destruction of this array and Context in general means more or less termination
        //               of the program and it may be unwise to delay exit with unnecessary computation.
        Array<void*> comp_arrs;
    };

    struct AssetContext {
        memory::LinkedListAllocator asset_metadata_allocator;

        // NOTE(kacper): should you do the same thing as with comp_arrs here?
        ArrayMap<AssetHandle, Pair<Mesh, AssetMetadata>>          meshes;
        ArrayMap<AssetHandle, Pair<Material, AssetMetadata>>      materials;
        ArrayMap<AssetHandle, Pair<Texture2D, AssetMetadata>>     textures;
        ArrayMap<AssetHandle, Pair<Cubemap, AssetMetadata>>       cubemaps;
        ArrayMap<AssetHandle, Pair<ShaderProgram, AssetMetadata>> shader_programs;

        StringArena asset_paths;
        StringArena shader_paths;
    };

    struct Context :
        AssetContext,
        EntityContext,
        RenderContext,
        platform::PlatformContext
    {
        int test = 12;
        memory::LinkedListAllocator memory;
        memory::LinkedListAllocator gp_debug_strings_allocator;
        Array<StringView> cmdline;
        char ** argv;
        s32 argc;

        ArrayMap<u32, Archive> open_archives;

        // preservation of client data when hot-swapping
        void ** client_preserved; 
        u64 client_preserved_size; 

        ivec2 window_size;

        StringView clientlib_path;
        char * cwd_path = nullptr;
        char * exe_path = nullptr;

        Channel<KeyEvent> key_input_channel;
        Channel<MouseMoveEvent> mouse_move_input_channel;
        Channel<MouseButtonEvent> mouse_button_input_channel;

        char key_state[32];

        Bitfield<debug::Category> stdout_log_categories = debug::category::all;
        debug::Level stdout_log_level = debug::level::all;

        bool exit_sig = false;

        platform::Clock clock;
    };
}
