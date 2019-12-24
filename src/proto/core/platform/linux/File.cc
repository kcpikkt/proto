#include "proto/core/platform/File.hh"
#include <fcntl.h>

namespace proto {
namespace platform {

    Err<FileErrCategory> File::open(StringView filepath, Mode mode) {
        const char * mode_str;
        switch(mode) {
            case (read_mode):
                mode_str = "rb"; break;
            case (write_mode):
                mode_str = "wb"; break;
            case (read_mode | write_mode):
                mode_str = "wb+"; break;
            default:
                assert(0 && "unsupported fileopen mode, implement me");
        }

        _file_ptr = fopen(filepath, mode_str);
        if(!_file_ptr) return FileErrCategory::open_fail;

        return FileErrCategory::success;
    }

    u64 File::size() {
        if(_cached_size) return _cached_size;

        fseek (_file_ptr , 0 , SEEK_END);
        u64 ret = ftell(_file_ptr);
        rewind (_file_ptr);

        return (_cached_size = ret);
    }

    u64 File::write(void const * buf, u64 size) {
        return fwrite(buf, sizeof(u8), size, _file_ptr);
    }

    u64 File::read(void * buf, u64 size) {
        return fread(buf, sizeof(u8), size, _file_ptr);
    }

    Err<FileErrCategory> File::reserve(u64 size) {
        int file_desc = fileno(_file_ptr);
        if(file_desc == -1) return FileErrCategory::desc_retrieve_fail;

        if(posix_fallocate(file_desc, 0, size))
            return FileErrCategory::reserve_fail;

        return FileErrCategory::success;
    }

    Err<FileErrCategory> File::close() {
        assert(_file_ptr);
        return (fclose(_file_ptr)
                ? FileErrCategory::close_fail
                : FileErrCategory::success);
    }

} // namespace platform
} // namespace proto
