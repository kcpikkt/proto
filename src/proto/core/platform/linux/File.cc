#include "proto/core/platform/File.hh"
#include "proto/core/util/defer.hh"
#include "proto/core/debug/logging.hh"
#include <fcntl.h>

namespace proto {
namespace platform {

    FileErr File::open(StringView filepath, Mode mode) {
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
        if(!_file_ptr)
            return FileErrCategory::open_fail;

        return FileErrCategory::success;
    }

    u64 File::size() {
        auto _cursor = cursor();
        defer{ seek(_cursor); };
        return (seek_end(), cursor());
    }

    u64 File::write(void const * buf, u64 size) {
        return fwrite(buf, sizeof(u8), size, _file_ptr);
    }

    u64 File::write(MemBuffer buf) {
        return write(buf.data, buf.size);
    }

    u64 File::read(void * mem, u64 size) {
        return fread(mem, sizeof(u8), size, _file_ptr);
    }

    u64 File::read(MemBuffer buf) {
        return read(buf.data, buf.size);
    }

    FileErr File::seek(s64 offset) {
        return fseek(_file_ptr, offset, SEEK_SET)
            ? FileErrCategory::seek_fail
            : FileErrCategory::success;
    }

    FileErr File::seek_end(s64 offset) {
        return fseek(_file_ptr, offset, SEEK_END)
            ? FileErrCategory::seek_fail
            : FileErrCategory::success;
    }

    s64 File::cursor() {
        return ftell(_file_ptr);
    }

    FileErr File::reserve(u64 size) {
        int file_desc = fileno(_file_ptr);
        if(file_desc == -1) return FileErrCategory::desc_retrieve_fail;

        if(posix_fallocate(file_desc, 0, size))
            return FileErrCategory::reserve_fail;

        return FileErrCategory::success;
    }

    FileErr File::close() {
        assert(_file_ptr);
        return (fclose(_file_ptr)
                ? FileErrCategory::close_fail
                : FileErrCategory::success);
    }

} // namespace platform
} // namespace proto
