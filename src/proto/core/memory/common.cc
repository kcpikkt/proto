#include "proto/core/memory/common.hh"

inline void * operator new(size_t sz, proto::memory::Allocator & allocator) {
    return allocator.alloc(sz);
}
