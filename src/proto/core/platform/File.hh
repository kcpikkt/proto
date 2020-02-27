#pragma once
#include "proto/core/common.hh"
#include "proto/core/error-handling.hh"
#include "proto/core/util/Bitfield.hh"
#include "proto/core/util/StringView.hh" 

#if defined(PROTO_PLATFORM_WINDOWS)
    #error not implemented
#elif defined(PROTO_PLATFORM_MAC)
    #error not implemented
#else 
# include <stdio.h> // FILE
#endif

namespace proto {
namespace platform {

struct File {

    enum Mode : u8 {
        read_mode   = BIT(0),
        write_mode  = BIT(1),
        overwrite_mode  = BIT(2),

        read_write_mode  = BIT(0) | BIT(1),
        read_overwrite_mode  = BIT(0) | BIT(2),
        //trunc_mode  = BIT(2),
        //create_mode = BIT(3),
        //append_mode = BIT(4),
    };
    //using Mode = u8;

    #if defined(PROTO_PLATFORM_WINDOWS)
        #error not implemented
    #elif defined(PROTO_PLATFORM_MAC)
        #error not implemented
    #else 
        FILE * _file_ptr = nullptr;
    #endif
    enum : u8 {
               mapped_bit = BIT(0),
    };
    Bitfield<u8> flags;
    //constexpr static u8 is_open_bit = BIT(0);

    Err open(StringView filename, Mode mode);
    u64 size();
    u64 write(void const * buf, u64 size); 
    u64 write(MemBuffer buf);

    u64 read(void * mem, u64 size); 
    u64 read(MemBuffer buf); 
    void flush();

    Err seek(s64 offset);
    Err seek_end(s64 offset = 0);

    s64 cursor();

    MemBuffer map(Mode mode, Span<u64> span = {0,0});
    void unmap(MemBuffer mem);

    Err resize(u64 size);
    Err reserve(u64 size);
    Err close();
};

} // namespace platform 
} // namespace proto 
