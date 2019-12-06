#pragma once
#include "proto/core/asset-system/common.hh"

namespace proto {

struct Cubemap : Asset {
    u8 channels;
    u32 gl_id;
    s32 bound_unit = -1;
    ivec2 size;
    void * data[6];
    Bitfield<u8> flags;
    constexpr static u8 gpu_uploaded_bit = BIT(1);
    constexpr static u8 bound_bit        = BIT(2);
    void init() {
        glGenTextures(1, &gl_id);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, gl_id);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

};

} // namespace proto
