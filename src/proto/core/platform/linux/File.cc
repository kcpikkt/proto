#include "proto/core/platform/File.hh"
#include "proto/core/util/defer.hh"
#include "proto/core/debug/logging.hh"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace proto {
namespace platform {

    FileErr File::open(StringView filepath, Mode mode) {
        const char * mode_str;
        switch(mode) {
            case read_mode:
                mode_str = "rb"; break;
            case write_mode:
            case overwrite_mode:
                mode_str = "wb"; break;
            case (read_write_mode):
                mode_str = "rb+"; break;
            case (read_mode | overwrite_mode):
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

    void File::flush() {
        fflush(_file_ptr); return;
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

    FileErr File::resize(u64 size) {
        int fd = fileno(_file_ptr);
        if(fd == -1) return FileErrCategory::desc_retrieve_fail;

        if(ftruncate(fd, size)) 
            return FileErrCategory::resize_fail;

        return FileErrCategory::success;
    }

    FileErr File::reserve(u64 size) {
        int file_desc = fileno(_file_ptr);
        if(file_desc == -1) return FileErrCategory::desc_retrieve_fail;

        if(posix_fallocate(file_desc, 0, size))
            return FileErrCategory::reserve_fail;

        return FileErrCategory::success;
    }

    MemBuffer File::map(File::Mode mode, Range range) {
        s32 map_prot, map_flags, fd;

        if(range.size == 0) {
            assert(range.begin == 0);
            range.size = size();
        }

        assert(range.begin <= size());

        if(range.size + range.begin > size())
            if(resize(range.size + range.begin)) return {};

        if(flags.check(mapped_bit)) {
            debug_warn(debug::category::main,
                       "Tried to map file to memory second time (that should be ok but unsupported), no operation performed.");
            return {};
        }

        fd = fileno(_file_ptr);
        if(fd == -1) return {};

        map_flags = MAP_SHARED;

        switch(mode) {
        case read_mode:
            map_prot = PROT_READ; break;
        case write_mode:
            map_prot = PROT_WRITE; break;
        case read_write_mode:
            map_prot = PROT_READ | PROT_WRITE; break;
        default:
            debug_warn(debug::category::main, "Unsupproted file mapping mode.");
            return {};
        }

        void * mapping = mmap(NULL, range.size, map_prot, map_flags, fd, range.begin);

        if(mapping != MAP_FAILED) {
            flags.set(mapped_bit);
            return {{mapping}, range.size};
        }

        return {};
    }

    // mem supplied has to be the same as obtained from map, hmmm, should i save it in File?
    void File::unmap(MemBuffer mem) {
        if(!flags.check(mapped_bit))
            return (void)debug_warn(debug::category::main,
                                    "Tried to unmap file that is not mapped, no operation performed.");
 
        assert(mem);
        flags.unset(mapped_bit);
        munmap(mem.data, mem.size);
    }

    FileErr File::close() {
        assert(_file_ptr);

        if(flags.check(mapped_bit))
            debug_warn(debug::category::main, "Closing memory mapped file that wasn't unmapped");

        return (fclose(_file_ptr)
                ? FileErrCategory::close_fail
                : FileErrCategory::success);
    }

} // namespace platform
} // namespace proto
