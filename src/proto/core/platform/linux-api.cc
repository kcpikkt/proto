#include "proto/core/platform/common.hh"
#if defined(PROTO_PLATFORM_LINUX) 

#include <libgen.h>
#include "proto/core/platform/api.hh"
#include "proto/core/memory.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/containers/StringArena.hh"
#include "proto/core/context.hh"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>

namespace proto {
namespace platform { 

void * alloc(size_t size) {
    //FIXME(kacper): do not use libc
    return malloc(size);
}

void free(void * mem) {
    //FIXME(kacper): do not use libc
    ::free(mem); 
}

const char * basename_substr(const char * path, u32 * no_ext_len) {
    const char * basename = path;

    size_t index = 0;
    while(*(path + index) != '\0') {
        char c = *(path + index);
        if(c == '/') { basename = path + index + 1; }
        index++;
    }
    if(no_ext_len) {
        *no_ext_len = (path + index) - basename;
        index = 0;
        // what about multiple file extensions?
        char c;
        while((c = basename[index]) != '\0') {
            if(c == '.') *no_ext_len = index;
            index++;
        }
    }
    return basename;
}

StringView basename_view(StringView path) {
    u32 index = 0;
    const char * basename;
    char c;
    while(c = path[index],
          index < path.length() && c != '\0')
    {
        // no backslashes in names, ok?
        if(c == '/' || c == '\\') { basename = path + index + 1; }
        index++;
    }
    if(index != path.length())
        debug_warn(debug::category::main,
                   "StringView contains null character on index "
                   "less than its length.", index, " ", path.length());

    u32 basename_offset = basename - path;
    u32 len = path.length() - basename_offset;

    index = path.length() - basename_offset;
    // what about multiple file extensions?
    while(c = basename[index], index >= 0) {
        if(c == '.') {
            len = index;
            break;
        }
        index--;
    }
    return StringView(basename, len);
}

// do I really need this function instead of just dirname_len?
const char * dirname_substr(const char * path, u32 * len) {
    if(len) *len = dirname_len(path);
    return path;
}

StringView dirname_view(const char * path) {
    return StringView(path, dirname_len(path));
}



u32 dirname_len(const char * path) {
    size_t index = strlen(path);
    while(--index != 0)
        if(path[index] == '/') return index;

    return 0;
}

const char * extension_substr(const char * path) {
    // basename is a macro for something
    size_t len = strlen(path) + 1;
    const char * extension = path + len - 1;

    s32 index = len - 1;
    while(index >= 0) {
        char c = *(path + index);
        if(c == '.') {
            extension = path + index + 1;
            return extension;
        }
        index--;
    }
    return extension;
}

StringView extension_view(const char * path) {
    return StringView(extension_substr(path));
}


#include "proto/core/io.hh"
// case insensitive
int strcmp_i(const char * str1, const char * str2) {
    for(u32 i=0; true; i++, str1++, str2++) {
        int diff = *str1 - *str2;
        if(diff != 0) {
                 if(*str1 >= 'a' && *str1 <= 'z' && diff ==  32) continue;
            else if(*str1 >= 'A' && *str1 <= 'Z' && diff == -32) continue;
            else return diff;
        }
        else if(*str1 == '\0') break;
    }
    return 0;
}

int strncmp_i(const char * str1, const char * str2, u32 n) {
    for(u32 i=0; i<n; i++, str1++, str2++) {
        int diff = *str1 - *str2;
        if(diff != 0) {
                 if(*str1 >= 'a' && *str1 <= 'z' && diff ==  32) continue;
            else if(*str1 >= 'A' && *str1 <= 'Z' && diff == -32) continue;
            else return diff;
        }
        else if(*str1 == '\0') break;
    }
    return 0;
}

//bool access(StringView path, FileModeType mode) {
//    return (access(path, F_OK) != -1)
//}

bool is_directory(StringView path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) return false;
    return S_ISDIR(statbuf.st_mode);
}

// struct File
// it does not allocate memory soo maybe idk
//File::File(){}
//
//File::File(memory::Allocator * allocator){
//    init(allocator);
//}
//
//int File::init(memory::Allocator * allocator){
//    assert(allocator);
//    _allocator = allocator;
//    return 0;
//}
StringArena ls(StringView dirpath) {
    StringArena arena;

    if(!is_directory(dirpath)) {
        debug_warn(debug::category::io,
                   "Path ", dirpath, " passed to ", __func__,
                   "is not directory");
        return arena;
    }
    char dirpath_cstr[256];
    strview_copy(dirpath_cstr, dirpath);

    DIR *d;
    struct dirent *dir;

    u32 arena_cap = 0;
    d = opendir(dirpath_cstr);
    if(d) {
        while((dir = readdir(d)) != NULL)
            arena_cap += strlen(dir->d_name) + 1;
        closedir(d);
    }

    arena.init(arena_cap, &context->gp_string_allocator);
    d = opendir(dirpath_cstr);
    if(d) {
        while((dir = readdir(d)) != NULL)
            arena.store(dir->d_name);
        closedir(d);
    }
    return arena;
}

const char *
path_ncat(char * dest,
                           StringView src,
                           u32 num)
{
    if(num == 0) return 0;

    u32 destlen = strlen(dest);

    if(src[0] == '/')
        debug_warn(proto::debug::category::main,
                   "appending an absolute path ", src, " to ", dest);

    // breaking the dest string for just a moment
    if(dest[destlen - 1] != '/' && src[0] != '/')
        dest[destlen++] = '/';

    u32 applen = min(src.length(), num);

    strncpy((dest + destlen), src, applen);

    // fixing string hersrclene
    // if outlen <= buflen then '\0' is already there
    dest[destlen + applen] = '\0';
    return dest;
}

int File::open(const char * filepath, FileModeType mode) {
    const char * mode_str;
    switch(mode) {
        case (file_read):
            mode_str = "rb"; break;
        case (file_write):
            mode_str = "wb"; break;
        case (file_read | file_write):
            mode_str = "wb+"; break;
        default:
            assert(0 && "unsupported fileopen mode");
    }
    file_ptr = fopen(filepath, mode_str);
    if(!file_ptr) return -1;
    file_desc = fileno(file_ptr);
    if(file_desc == -1) return -2;
    return 0;
}

u64 File::write(const void * buf, size_t size) {
    return fwrite(buf, sizeof(u8), size, file_ptr);
}

int File::read(void * buf, size_t size) {
    return fread(buf, sizeof(u8), size, file_ptr);
}

u64 File::size() {
    if(_cached_size) return _cached_size;

    fseek (file_ptr , 0 , SEEK_END);
    u64 ret = ftell (file_ptr);
    rewind (file_ptr);
    _cached_size = ret;
    return ret;
}

int File::reserve(size_t size) {
    //   assert(is_initialized);
    assert(file_ptr);
    assert(file_desc != -1);
    return posix_fallocate(file_desc, 0, size);
}
int File::close() {
    //assert(is_initialized);
    assert(file_ptr);
    return fclose(file_ptr);
}


} // namespace platform
} // namespace proto 

#endif
