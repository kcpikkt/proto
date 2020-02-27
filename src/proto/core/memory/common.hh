#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/debug/assert.hh"

namespace proto {
namespace memory{

    // NOTE(kacper):
    // dynamic polymorphism was particularly handy here
    // allocation calls are not so often to be a bottleneck
    // there are only few types of allocators so vtables are small
    struct Allocator {
        virtual void * alloc(u64) = 0;
        virtual void * realloc(void *, u64) = 0;
        virtual void   free(void *) = 0;
    };


    using byte = unsigned char;
    static u64 _def_align = 16;

    static constexpr u64 max_alignment = alignof(long double);

    static constexpr inline u64 kilobytes(u64 bytes) { return 1024ull * bytes; }
    static constexpr inline u64 kb       (u64 bytes) { return 1024ull * bytes; }

    static constexpr inline u64 megabytes(u64 bytes) { return 1024ull * kb(bytes); }
    static constexpr inline u64 mb       (u64 bytes) { return 1024ull * kb(bytes); }

    static constexpr inline u64 gigabytes(u64 bytes) { return 1024ull * mb(bytes); }
    static constexpr inline u64 gb       (u64 bytes) { return 1024ull * mb(bytes); }

    static constexpr bool is_pow2(u64 n) {
        return (n != 0) && !(n & (n-1));
    }

    static inline bool is_aligned(u64 off, u64 alignment = _def_align) {
        return ((off % alignment) == 0);
    }

    static inline bool is_aligned(void * addr, u64 alignment = _def_align) {
        return is_aligned((u64)addr, alignment);
    }


    // alginoff
    constexpr static inline u64 align_back(u64 offset, u64 alignment = _def_align) {
        assert(is_pow2(alignment));
        return offset & ~(alignment - 1);
    }

    constexpr static inline u64 align_forw(u64 offset, u64 alignment = _def_align) {
        return align_back(offset + alignment - 1, alignment);
    }

    constexpr static inline u64 align(u64 offset, u64 alignment = _def_align) {
        return align_forw(offset, alignment);
    }

    template<typename T>
    static void * align_forw(T * addr, u64 alignment = _def_align) {
        assert(is_pow2(alignment));
        return (void *)(  (u64)((byte*)addr + (alignment - 1)) &
                        ~((u64)(alignment - 1)) );
    }

    template<typename T>
    static void * align_back(T * addr, u64 alignment = _def_align) {
        assert(is_pow2(alignment));
        return align_forw((void*)((byte*)addr - (alignment - 1)),
                          alignment);
    }

    template<typename T>
    static void * align(T * addr, u64 alignment = _def_align) {
        return align_forw(addr, alignment);
    }



} // namespace memory
} // namespace proto

void * operator new(proto::u64 sz, proto::memory::Allocator & allocator);
