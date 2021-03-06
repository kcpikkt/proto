#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/platform/common.hh"
#include "proto/core/platform/macros.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/memory/common.hh"

#include "proto/core/platform/File.hh"
#include "proto/core/platform/Clock.hh"

namespace proto {
    struct String;
    struct StringArena;
namespace platform {
    const char* cwd(char * buf, size_t len);
    size_t cwd_length();

    char * dir_name(char * filepath);
    char * base_name(char * filepath);

    int print_f(const char * format, ...); 

    //TODO(kacper):
    // either make it more like windows VirtualAlloc or add
    // add VirtualAlloc functionality in form of separate calls
    void * alloc(size_t size);
    void free(void * mem);

    const char * basename_substr(const char * path, u32 * no_ext_len);
    const char * dirname_substr(const char * path, u32 * len);
    const char * extension_substr(const char * path);

    StringView basename_view(StringView path);
    StringView dirname_view(const char * path);
    StringView extension_view(const char * path);

    u32 dirname_len(const char * path);

    const char * path_ncat(char * dest, StringView src, u32 num);

    // TODO(kacper): move it to string.hh
    // case insensivite strcmp
    int strncmp_i(const char * str1, const char * str2, u32 n); 

    bool is_directory(StringView path);
    bool is_file(StringView path);

    String search_for_file(StringView filename, StringArena& dirs);

    // TODO(kacper): allocate inside, return unique_ptr to string arena;
    // TODO(kacper): ls_rel, ls_abs?
    StringArena ls(StringView dirpath);

} // namespace platform
} // namespace proto
