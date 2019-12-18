#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/util/Bitfield.hh"

namespace proto {
    struct Renderbuffer {
        u32 gl_id;
        u32 gpu_format = GL_DEPTH_COMPONENT;
        ivec2 size;
        Bitfield<u8> flags;
        constexpr static u8 on_gpu_bit = BIT(0);

        void init(ivec2 size, u32 gpu_format) {
            init();
            this->size = size;
            this->gpu_format = gpu_format;
        }

        void init() {
            glGenRenderbuffers(1, &gl_id);
        }

        Renderbuffer& $_init(ivec2 size, u32 gpu_format) {
            init(size, gpu_format); return *this;
        }
    };

} // namespace proto
