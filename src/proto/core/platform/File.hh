#pragma once
#include "proto/core/error-handling.hh"
#include "proto/core/util/Bitfield.hh"

namespace proto {
namespace platform {

struct FileErrCategory : ErrCategoryCRTP<FileErrCategory> {
    enum {
          success = 0,
          open_fail,
          desc_retrieve,
          wrong_mode,
          desc_retrieve_fail,
          reserve_fail,
          close_fail
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
        case reserve_fail:
            return "Failed to retrieve file descriptor of a file.";
        default:
            return "no error message";
        }
    }
};

struct File
{
    using Mode = u8;
    constexpr static Mode read_mode   = BIT(0);
    constexpr static Mode write_mode  = BIT(1);
    constexpr static Mode trunc_mode  = BIT(2);
    constexpr static Mode create_mode = BIT(3);
    constexpr static Mode append_mode = BIT(4);

    #if defined(PROTO_PLATFORM_WINDOWS)
        #error not implemented
    #elif defined(PROTO_PLATFORM_MAC)
        #error not implemented
    #else 
        FILE * _file_ptr = nullptr;
    #endif

    u64 _cached_size = 0;
    Bitfield<u8> flags;
    constexpr static u8 is_open_bit = BIT(0);

    Err<FileErrCategory> open(StringView filename, Mode mode);
    u64 size();
    u64 write(void const * buf, u64 size); 
    u64 read(void * buf, u64 size); 
    Err<FileErrCategory> reserve(u64 size);
    Err<FileErrCategory> close();
};

} // namespace platform 
} // namespace proto 
