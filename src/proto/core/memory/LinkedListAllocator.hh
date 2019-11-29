#pragma once
#include "proto/core/common.hh"
#include "proto/core/common/types.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/debug/markers.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/meta.hh"

namespace proto {
namespace memory {
    // This allocator keeps single linked, sorted, list of all nodes.
    // as well as linked list of all nodes in the same order
    // as they appear in memory.
    // Each node consists of a header and block of memory in this order.
    // The latter is by default aligned with max alignment (usually 16 bytes).
    // Because of that and hefty (32 byte) headers it is rather wasteful
    // to use this allocator for tiny things.
    // Consider proto::memory::strings_allocator if you need allocator for c-strings.

    // NOTE: Though it max aligns by default it may still consist of
    //       alloc method with alignment parameter just for compaitibility,
    //       keep in mind that this paramter is ignored.

    struct LinkedListAllocator : Allocator, PROTO_DEBUG_MARKER
    {

        constexpr static byte header_magic_number = 0xAF;
        struct Header {
            constexpr static byte FREE = 1;

            // spare 8 bytes can be used as overflow&free safety mechanisms

            // primitive safety mechanism
            byte magic_number;
            // another, less primitive, safety mechanism
            byte first_addr_byte = 0;

            byte flags = 0;
            Header * next = nullptr;
            Header * next_in_mem = nullptr;
            size_t size = 0;
        };
        static_assert(sizeof(Header) == 32);

        Header * _first = nullptr;
        Header * _first_in_mem = nullptr;
        Header * _last = nullptr;

        void * _arena = nullptr;
        size_t _size = 0;
        size_t _used = 0;
        size_t _alloc_count = 0;
        size_t _min_block_size = 32; // FIXME
        size_t _min_alloc_size = 32; // FIXME

        void * raw();

        int init(size_t init_size);

        int init(void * _mem, size_t size);

        int init(Allocator * other_allocator, size_t size);

        [[nodiscard]]
        void * alloc(size_t requested_size);
        [[nodiscard]]
        void * alloc(size_t requested_size, size_t alignment);

        [[nodiscard]]
        void * realloc(void * block, size_t requested_size);
        [[nodiscard]]
        void * realloc(void * block, size_t requested_size, size_t alignment);

        void free(void * block);

        void unlink_node(Header * node, Header * prev = nullptr);

        bool is_linked(Header * node);

        void insert_sort(Header * node);

        void insert_sort(Header * fst, Header * snd);

        Header * find_free_node(size_t requested_size,
                                Header ** prev_node = nullptr);

        Header * merge_with_next_in_mem (Header * node);

        Header * 
        try_split(Header * node, size_t needed_size, Header * prev_node = nullptr);

        Header * get_header(void * addr);

        void * get_block(Header * header_addr);

        u8 get_block_padding(Header * header_addr);

#if defined(PROTO_DEBUG)
        void sanity_check();
        size_t addr_offset(void * addr);
        //#if defined(PROTO_VERBOSE)
        void debug_print();
        //#endif
#endif

    private: 
        void emplace_init_node();
    };
} // namespace memory
} // namespace proto

namespace proto::meta {
    template<>
    struct is_allocator<proto::memory::LinkedListAllocator> : true_t {};
}

