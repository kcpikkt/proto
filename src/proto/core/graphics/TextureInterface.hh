// NOTE(kacper): nothing fancy here,
//               just simple data interface
//               to shrink texture binding code

#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/util/Bitfield.hh"

namespace proto {

struct TextureInterface : Asset {
    u8 channels; // deprecate, use format instead
    
    u32 format;
    u32 gpu_format;

    u32 gl_id;
    s32 bound_unit = -1;
    ivec2 size;
    Bitfield<u8> flags = 0;
    constexpr static u8 on_gpu_bit = BIT(0);
    constexpr static u8 bound_bit  = BIT(1);

    // actually cpy
    void _move(TextureInterface&& other) {
        channels   = other.channels;
        gl_id      = other.gl_id;
        bound_unit = other.bound_unit;
        size       = other.size;
        flags      = other.flags;
    }
};

} // namespace proto
