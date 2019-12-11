#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/platform/common.hh"
#include "proto/core/platform/macros.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/memory/common.hh"

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

    String search_for_file(StringArena& dirs, StringView filename);

    // TODO(kacper): allocate inside, return unique_ptr to string arena;
    // TODO(kacper): ls_rel, ls_abs?
    StringArena ls(StringView dirpath);

    using FileModeType = u8;
    constexpr static FileModeType file_read   = BIT(1);
    constexpr static FileModeType file_write  = BIT(2);
    constexpr static FileModeType file_trunc  = BIT(3);
    constexpr static FileModeType file_create = BIT(4);
    constexpr static FileModeType file_append = BIT(5);

    struct File {
        size_t _cached_size = 0;
        u8 flags = 0;

        constexpr static u8 IS_INITIALIZED = 1;
        constexpr static u8 IS_OPEN = 2;

        inline bool is_open() {
            return (flags & IS_OPEN);
        }
        inline bool is_initialized() {
            return (flags & IS_INITIALIZED);
        }

        //File();
        //File(memory::Allocator * allocator);
        //int init(memory::Allocator * allocator);
        int open(const char * path, FileModeType mode);
        size_t file_size();
        int close();
        u64 size();
        u64 write(const void * buf, size_t size); 
        int read(void * buf, size_t size); 
        int reserve(size_t size);

        //Filepath path;
    #if defined(PROTO_PLATFORM_WINDOWS)
        #error not implemented
    #elif defined(PROTO_PLATFORM_MAC)
        #error not implemented
    #else 
        FILE * file_ptr = nullptr;
        int file_desc = -1;
    #endif
    };


} // namespace platform
} // namespace proto
