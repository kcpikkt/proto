#include "proto/core/platform/File.hh"
#include "proto/core/util/defer.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/common/types.hh"

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace proto {
namespace platform {

    Err File::open(StringView filepath, Mode mode) {
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
            return IO_FILE_OPEN_ERR;

        return SUCCESS;
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

    Err File::seek(s64 offset) {
        return fseek(_file_ptr, offset, SEEK_SET)
            ? IO_FILE_SEEK_ERR
            : SUCCESS;
    }

    Err File::seek_end(s64 offset) {
        return fseek(_file_ptr, offset, SEEK_END)
            ? IO_FILE_SEEK_ERR
            : SUCCESS;
    }

    s64 File::cursor() {
        return ftell(_file_ptr);
    }

    Err File::resize(u64 size) {
        int fd = fileno(_file_ptr);
        if(fd == -1) return IO_FILE_ERR;

        if(ftruncate(fd, size)) 
            return IO_FILE_ERR;

        return SUCCESS;
    }

    Err File::reserve(u64 size) {
        int file_desc = fileno(_file_ptr);
        if(file_desc == -1) return IO_FILE_ERR;

        if(posix_fallocate(file_desc, 0, size))
            return IO_FILE_RESERVE_ERR;

        return SUCCESS;
    }

    MemBuffer File::map(File::Mode mode, Span<u64> span) {
        s32 map_prot, map_flags, fd;

        // default mapping whole file
        if(span.size == 0) {
            assert(span.begin == 0);
            span.size = size();
        }

        assert(span.begin <= size());

        if(span.size + span.begin > size())
            if(resize(span.size + span.begin)) return {};

        if(flags.check(mapped_bit)) {
            debug_warn(debug::category::main,
                       "Tried to map file to memory second time "
                       "(that should be ok but unsupported), no operation performed.");
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

        void * mapping = mmap(NULL, span.size, map_prot, map_flags, fd, span.begin);

        if(mapping != MAP_FAILED) {
            flags.set(mapped_bit);
            return {{mapping}, span.size};
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

    Err File::close() {
        assert(_file_ptr);

        if(flags.check(mapped_bit))
            debug_warn(debug::category::main, "Closing memory mapped file that wasn't unmapped");

        return (fclose(_file_ptr)
                ? IO_FILE_CLOSE_ERR 
                : SUCCESS);
    }

} // namespace platform
} // namespace proto
