#pragma once
#include "proto/core/memory/common.hh"
#include "proto/core/meta.hh"
#include <assert.h>

namespace proto::memory
{
    struct linear_allocator
    {
        void * _cursor = nullptr;
        void * _arena  = nullptr;
        size_t _size = 0;
        size_t _used = 0;
        size_t _alloc_count = 0;

        struct header {
            int size;
        };

        int init(size_t init_size)
        {
            assert(_arena == nullptr and
                   _cursor == nullptr and
                   _size  == 0);

            _arena = (void*) malloc(init_size);
            if(_arena) {
                assert(is_aligned(_arena, 16));
                _size = init_size;
                _cursor = _arena;
                return 0;
            } else {
                return 1;
            }
        }
        void release() {
            free(_arena);
            //TODO(kacper): debug warning
            _arena = nullptr;
            _cursor = nullptr;
            _size = 0;
            _used = 0;
            _alloc_count = 0;
        }

        void * alloc(size_t sz, size_t alignment)
        {
            // assert(is_power_of_two(alignment));

            assert(_arena != nullptr and
                   _cursor != nullptr and
                   _size  != 0);

            byte * bcursor = (byte*)_cursor;
            void * header_ptr;
            void * alloc_block_ptr;

            header_ptr =
                align_forw((void*)bcursor, alignof(header));
            bcursor = (byte*)header_ptr + sizeof(header);

            alloc_block_ptr =
                align_forw((void*)bcursor, alignment);
            bcursor += sz;

            //NOTE(kacper): so that header is always at first
            // header-aligned address preceding the block and its easy to find it
            header_ptr = align_back(alloc_block_ptr, alignof(header));

            assert(bcursor < (byte*)_arena + _size);

            if (bcursor < (byte*)_arena + _size) {
                ((header*)header_ptr)->size = sz;

                _cursor = bcursor;

                return alloc_block_ptr;
            } else
                return nullptr;
        }

        header extract_header(void * addr)
        {
            return *((header*) align_back(addr, alignof(header)));
        }
    };
}

namespace proto::meta {
    template<>
    struct is_allocator<proto::memory::linear_allocator> : true_t {};
}

