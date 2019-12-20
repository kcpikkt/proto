#pragma once
#include <stdlib.h>
#include "proto/core/memory/common.hh"

namespace proto {
namespace platform {

struct OSAllocator : memory::Allocator {

    [[nodiscard]]
    void * alloc(u64 requested_size) {
        return ::malloc(requested_size);
    }

    [[nodiscard]]
    void * realloc(void * ptr, u64 requested_size) {
        return ::realloc(ptr, requested_size);
    }

    void free(void * ptr) {
        ::free(ptr);
    }
};

} // namespace platform
} // namespace proto
