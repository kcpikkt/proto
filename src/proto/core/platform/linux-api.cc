#include "proto/core/platform/common.hh"
#if defined(PROTO_PLATFORM_LINUX) 

#include <libgen.h>
#include "proto/core/platform/api.hh"
#include "proto/core/memory.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/containers/StringArena.hh"
#include "proto/core/util/String.hh"
#include "proto/core/context.hh"
#include "proto/core/io.hh"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>

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
    const char * basename = path.str;
    char c;
    while(index < path.length) {
        c = path[index];
        if(c == '\0') break;
        // no backslashes in names, ok?
        if(c == '/' || c == '\\') { basename = path.str + index + 1; }
        index++;
    }
    if(index != path.length)
        debug_warn(debug::category::main,
                   "StringView contains null character on index "
                   "less than its length.", index, " ", path.length);

    u32 basename_offset = basename - path.str;
    u32 len = path.length - basename_offset;

    index = path.length - basename_offset;
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
//FIXME(kacper): empty string crash
//NOTE(kacper):  btw, write tests for them for gods sake

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

bool is_file(StringView path) {
    struct stat statbuf;
    if (stat(path, &statbuf) != 0) return false;
    return S_ISREG(statbuf.st_mode);
}

String search_for_file(StringView filename, StringArena& dirs) {
    String ret;

    static char filepath[PROTO_MAX_PATH];
    strview_copy(filepath, filename);

    if(access(filepath, F_OK) == -1) {
        for(u32 i=0; i < dirs.count(); i++) {
            if(!is_directory(dirs[i])) continue;

            strview_copy(filepath, dirs[i]);

            platform::path_ncat(filepath, filename, 512);

            if(access(filepath, F_OK) != -1) {
                ret.init(filepath, &context->memory);
                break;
            }
        }
    } else {
        //TOOD(kacper): use proper(er) allocator for that
        ret.init(filename, &context->memory);
    }
    return meta::move(ret);
}

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

    arena.init(arena_cap, &context->memory);
    set_debug_marker(arena, "sys::ls() return arena");
    d = opendir(dirpath_cstr);
    if(d) {
        while((dir = readdir(d)) != NULL)
            arena.store(dir->d_name);
        closedir(d);
    }
    return arena;
}


    //void watch_file(StringView filepath, void(callback*)()) {
    //    if(proto::context->inotify_fd == -1) {
    //        debug_error(debug::category::main,
    //                    "Cannot watch file ", path,
    //                    ", inotify instance not initialized.");
    //        return;
    //    }
    //    
    //    static char _filepath[PROTO_MAX_PATH];
    //    strview_copy(_filepath, filepath);
    //
    //    int watch_fd =
    //        inotify_add_watch(_context.inotify_fd, _filepath, IN_MODIFY);
    //
    //    if(watch_fd == -1) {
    //        debug_error(debug::category::main,
    //                    "inofify_add_watch() failed for file ", filepath);
    //        return;
    //    }
    //
    //    context->watched_files.push_back(watch_fd);
    //}

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

    u32 applen = min(src.length, num);

    strncpy((dest + destlen), src, applen);

    // fixing string hersrclene
    // if outlen <= buflen then '\0' is already there
    dest[destlen + applen] = '\0';
    return dest;
}


} // namespace platform
} // namespace proto 

#endif
