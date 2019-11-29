#pragma once
#include "proto/core/common/types.hh"

namespace proto {

template<typename T>
struct Buffer {
    union {
        void * data;
        u8   * data8;
        u16  * data16;
        u32  * data32;
        u64  * data64;
    };
    size_t size;
};

using MemBuffer = Buffer<void>;
using StrBuffer = Buffer<char>;

} //namespace proto 
