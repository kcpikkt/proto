// NOTE(kacper): nothing fancy here,
//               just simple data interface
//               to shrink texture binding code

#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/util/Bitfield.hh"

namespace proto {

struct TextureInterface : Asset {
    u8 channels;
    u32 gl_id;
    s32 bound_unit = -1;
    ivec2 size;
    Bitfield<u8> flags;
    constexpr static u8 gpu_uploaded_bit = BIT(1);
    constexpr static u8 bound_bit        = BIT(2);

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
