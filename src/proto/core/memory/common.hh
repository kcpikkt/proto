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
        virtual void * alloc(proto::u64) = 0;
        virtual void * realloc(void *, proto::u64) = 0;
        virtual void   free(void *) = 0;
    };


    using byte = unsigned char;

    static constexpr proto::u64 max_alignment = alignof(long double);

    static constexpr proto::u64 kilobytes(proto::u64 bytes) {
        return 1024ull * bytes;
    }
    static constexpr proto::u64 megabytes(proto::u64 bytes) {
        return 1024ull * kilobytes(bytes);
    }
    static constexpr proto::u64 gigabytes(proto::u64 bytes) {
        return 1024ull * megabytes(bytes);
    }
    static constexpr bool is_power_of_two(proto::u64 n) {
        return (n != 0) && !(n & (n-1));
    }

    static bool is_aligned(void * addr, proto::u64 alignment) {
        return ((proto::u64)addr % alignment == 0);
    }

    template<typename T>
    static void * align_forw(T * addr, proto::u64 alignment) {
        assert(is_power_of_two(alignment));
        return (void *)(  (proto::u64)((byte*)addr + (alignment - 1)) &
                        ~((proto::u64)(alignment - 1)) );
    }

    template<typename T>
    static void * align_back(T * addr, proto::u64 alignment) {
        assert(is_power_of_two(alignment));
        return align_forw((void*)((byte*)addr - (alignment - 1)),
                          alignment);
    }

} // namespace memory
} // namespace proto

void * operator new(proto::u64 sz, proto::memory::Allocator & allocator);
