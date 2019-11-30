// header should be already precompiled with that flag
#define CATCH_CONFIG_COLOUR_NONE
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
//#define PROTO_DEBUG
#include <proto/core/memory/LinkedListAllocator.hh>
#include <proto/core/io.hh>

TEST_CASE("LinkedListAllocator") {
    using namespace proto;
    using namespace proto::memory;
        
    // not much to test here since allocators have
    // online sanity checks tests while in debug mode

    size_t asize = 1024;
    LinkedListAllocator a;
 
    // for visualization
    auto fill = [&](void * mem, size_t size, unsigned char c) -> void {
                    assert(mem);


                    for(size_t i=0; i<size; i++)
                        *((unsigned char*)mem + i) = c;
                };
    
    auto fillalloc = [&](size_t size, char c) -> void * {
                         void * ret = a.alloc(size, 16);
                         if(ret)
                            fill(ret, a.get_header(ret)->size, c);
                         return ret;
                     };

    auto fillrealloc =
        [&](void * mem, size_t size, char c_free, char c_new) -> void *{
                         assert(mem);
                         fill(mem, a.get_header(mem)->size, c_free);
                         void * ret = a.realloc(mem, size);
                         puts("after");
                         if(ret)
                            fill(ret, a.get_header(ret)->size, c_new);
                         else 
                            fill(mem, a.get_header(mem)->size, c_new);
                         return ret;
                     };

    auto fillfree = [&](void * mem, char c) -> void {
                         assert(mem);
                         fill(mem, a.get_header(mem)->size, c);
                         a.free(mem);
                     };

    auto print_mem = [&](void* mem, size_t size) {
                printf("\n");
                for(int i = 0; i<24; i++) printf("%.2d ", i);
                printf("\n");
                for(size_t i = 0; i<24; i++) printf("---");
                printf("\n");

                for(size_t i = 0; i<size; i++) {
                    if((i)%24 == 0) printf("%.3X  ", (unsigned int)i);
                    printf("%.2X ",
                           *((unsigned char*)mem + i));
                    if((i+1)%24 == 0) printf("\n");
                }
                printf("\n");
            };

    a.init(asize);
    fill(a.get_block(a._first), a._first->size, 0x00);

    // 0x00 - free init block
    // 0x*0 - free mem* block
    // 0x** - non-free mem* block
    // every header starts with 0xAF
    //    (more precisely with LikedListAllocator::header_magic_number)

    SECTION("") {
        void * mem1 = fillalloc(128, 0x11);
        void * mem2 = fillalloc(256, 0x22);
        void * mem3 = fillalloc(64, 0x33);
        fillfree(mem2, 0x20);
        // 96 + 32 = 128, should take place of previous mem2 block
        void * mem4 = fillalloc(96, 0x44);
        CHECK(mem4 == mem2);
        // ...and leave 160b for 32b header and 128b block
        CHECK(a.get_header(mem4)->next_in_mem->size == 128);

        // ...and it should be free
        CHECK((a.get_header(mem4)->next_in_mem->flags &
               LinkedListAllocator::Header::FREE));

        // too big, wont fit in this gap
        void * mem5 = fillalloc(256, 0x55);
        CHECK(a.get_header(mem5) != a.get_header(mem4)->next_in_mem);

        // allocating less than the block size but with residue less than
        // _min_block_size shouldn't split the block.
        auto * prev_next_in_mem = 
            a.get_header(mem4)->next_in_mem->next_in_mem;
        void * mem6 =
            fillalloc(a.get_header(mem4)->next_in_mem->size -
                      a._min_block_size,
                      0x66);
        // ...and therefore next_in_mem of mem6 should be the same as
        // next_in_mem of next_in_mem of mem4 before allocation
        CHECK(prev_next_in_mem == a.get_header(mem6)->next_in_mem);

        // by now we should have 160b block left at the end
        // allocating more should fail
        CHECK(fillalloc(161, 0xFF) == nullptr);

        void * mem7 = fillalloc(160, 0x77);
        CHECK(mem7);

        // allocator should be filled
        CHECK(a._used == a._size);

        // lets free 256b mem5 and realloc 64b mem3 into 128b
        // since mem3 is right behind mem5 in memory ot should expand into it

        fillfree(mem5, 0x50);
        mem3 = fillrealloc(mem3, 128, 0x30, 0x33);

        print_mem(a.raw(), asize);
    }


        /*
    SECTION("linked list order") {
        void * mem0 = fillalloc(128);
        void * mem1 = fillalloc(64);
        void * mem2 = fillalloc(159);
        void * mem3 = fillalloc(32);
        SECTION("blocks are sorted by size"){
            LinkedListAllocator::header * lookup = a._first;
            CHECK(a.get_block(lookup) == mem3);
            lookup = lookup->next;

            CHECK(a.get_block(lookup) == mem1);
            lookup = lookup->next;

            CHECK(a.get_block(lookup) == mem0);
            lookup = lookup->next;

            CHECK(a.get_block(lookup) == mem2);
            lookup = lookup->next;
        }

    }

        */

}
