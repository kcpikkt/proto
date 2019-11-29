#pragma once
#include <stddef.h>
#include <assert.h>

namespace proto {
namespace memory{

    // NOTE(kacper):
    // dynamic polymorphism was particularly handy here
    // allocation calls are not so often to be a bottleneck
    // there are only few types of allocators so vtables are small
    struct Allocator {
        virtual void * alloc(size_t) = 0;
        virtual void * realloc(void *, size_t) = 0;
        virtual void   free(void *) = 0;
    };


    using byte = unsigned char;

    static constexpr size_t max_alignment = alignof(long double);

    static constexpr size_t kilobytes(size_t bytes) {
        return 1024ull * bytes;
    }
    static constexpr size_t megabytes(size_t bytes) {
        return 1024ull * kilobytes(bytes);
    }
    static constexpr size_t gigabytes(size_t bytes) {
        return 1024ull * megabytes(bytes);
    }
    static constexpr bool is_power_of_two(size_t n) {
        return (n != 0) && !(n & (n-1));
    }

    static bool is_aligned(void * addr, size_t alignment) {
        return ((size_t)addr % alignment == 0);
    }

    template<typename T>
    static void * align_forw(T * addr, size_t alignment) {
        assert(is_power_of_two(alignment));
        return (void *)(  (size_t)((byte*)addr + (alignment - 1)) &
                        ~((size_t)(alignment - 1)) );
    }

    template<typename T>
    static void * align_back(T * addr, size_t alignment) {
        assert(is_power_of_two(alignment));
        return align_forw((void*)((byte*)addr - (alignment - 1)),
                          alignment);
    }

} // namespace memory
} // namespace proto

void * operator new(size_t sz, proto::memory::Allocator & allocator);
