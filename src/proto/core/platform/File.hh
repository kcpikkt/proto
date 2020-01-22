#pragma once
#include "proto/core/error-handling.hh"
#include "proto/core/util/Bitfield.hh"
#include "proto/core/util/Buffer.hh"
#include "proto/core/util/Range.hh" 

namespace proto {
namespace platform {

struct FileErrCategory : ErrCategoryCRTP<FileErrCategory> {
    enum {
          success = 0,
          open_fail,
          desc_retrieve,
          wrong_mode,
          desc_retrieve_fail,
          resize_fail,
          reserve_fail,
          close_fail,
          seek_fail,
    };
    static ErrMessage message(ErrCode code){
        switch(code) {
        case success:
            return "Success.";
        case open_fail:
            return "Failed to open file.";
        case wrong_mode:
            return "Wrong or unsupported file open mode bitflags.";
        case desc_retrieve_fail:
            return "Failed to retrieve file descriptor of a file.";
        case resize_fail:
            return "Failed to resize file descriptor of a file.";
        case reserve_fail:
            return "Failed to retrieve file descriptor of a file.";
        case seek_fail:
            return "Failed to seek into the file.";
        default:
            return "Unknown error";
        }
    }
};

using FileErr = Err<FileErrCategory>;

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

    FileErr open(StringView filename, Mode mode);
    u64 size();
    u64 write(void const * buf, u64 size); 
    u64 write(MemBuffer buf);

    u64 read(void * mem, u64 size); 
    u64 read(MemBuffer buf); 
    void flush();

    FileErr seek(s64 offset);
    FileErr seek_end(s64 offset = 0);

    s64 cursor();

    MemBuffer map(Mode mode, Range range = {0,0});
    void unmap(MemBuffer mem);

    FileErr resize(u64 size);
    FileErr reserve(u64 size);
    FileErr close();
};

} // namespace platform 
} // namespace proto 
