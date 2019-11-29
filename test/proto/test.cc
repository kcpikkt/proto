// header should be already precompiled with that flag
#define CATCH_CONFIG_COLOUR_NONE
#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"
#define PROTO_DEBUG
#include <proto/core/memory/LinkedListAllocator.hh>
#include <proto/core/io.hh>

TEST_CASE() {
    using namespace proto;
    using namespace proto::memory;
        
    // not much to test here since allocators have
    // online sanity check tests while in debug mode

    size_t asize = 1024;
    LinkedListAllocator a;
 
    // for visualization
    auto fill = [&](void * mem, size_t size, unsigned char c) -> void {
                    assert(mem);


                    for(int i=0; i<size; i++)
                        *((unsigned char*)mem + i) = c;
                };
    
    auto fillalloc = [&](size_t size) -> void * {
                         void * ret = a.alloc(size, 16);
                         assert(ret);
                         fill(ret, a.get_header(ret)->size, 0xFF);
                         return ret;
                     };

    auto fillfree = [&](void * mem) -> void {
                         assert(mem);
                         fill(mem, a.get_header(mem)->size, 0xAA);
                         a.free(mem);
                     };

    auto print_mem = [&](void* mem, size_t size) {
                printf("\n");
                for(int i = 0; i<24; i++) printf("%.2d ", i);
                printf("\n");
                for(size_t i = 0; i<24; i++) printf("---");
                printf("\n");

                for(size_t i = 0; i<size; i++) {
                    if((i)%24 == 0) printf("%.3X  ", i);
                    printf("%.2X ",
                           *((unsigned char*)mem + i));
                    if((i+1)%24 == 0) printf("\n");
                }
                printf("\n");
            };

    a.init(asize);
    fill(a.get_block(a._first), a._first->size, 0x11);
   
    SECTION("") {
        void * mem0 = fillalloc(128);
        void * mem1 = fillalloc(256);
        void * mem2 = fillalloc(64);
        fillfree(mem1);
        // 96 + 32 = 128, should take place of previous mem1 block
        void * mem3 = fillalloc(96);
        CHECK(mem3 == mem1);
        // ...and leave 128 size block
        CHECK(a.get_header(mem3)->next_in_mem->size == 128);

        // next after should be free
        CHECK((a.get_header(mem3)->next_in_mem->flags &
               LinkedListAllocator::Header::FREE));

        // too big, wont fit
        void * mem4 = fillalloc(256);
        CHECK(a.get_header(mem4) != a.get_header(mem3)->next_in_mem);


        // 64 + 32 (of next node header) = 96
        void * mem5 = fillalloc(64);
        CHECK(a.get_header(mem5) == a.get_header(mem3)->next_in_mem);

        //...which leave us with 32 left
        void * mem6 = fillalloc(32);
        CHECK(a.get_header(mem6) == a.get_header(mem5)->next_in_mem);

        // free realloc, the same node should be used
        fillfree(mem6);
        void * mem7 = fillalloc(32);
        CHECK(mem6 == mem7);

        // by now we should have 160 bytes left
        // allocating more should fail

        CHECK(a.alloc(161, 16) == nullptr);

        // we can realloc previously allocated 64 bytes block to 128 bytes.
        fill(mem5, a.get_header(mem5)->size, 0x22);
        // that should take the last 160 chunk
        mem5 = a.realloc(mem5, 128, 16);
        fill(mem5, a.get_header(mem5)->size, 0x33);
        CHECK(mem5);

        // now we should have some space for exapansion of block mem3
        CHECK(a.realloc(mem3, 192, 16));
        CHECK(a.get_header(mem3)->size >= 192);

        fill(mem3, a.get_header(mem3)->size, 0x44);

        a.sanity_check();

        // mem6 and mem2 are consecutive blocks, should be merged when freed
        // this should give us spare 128 bytes of memory...
        fillfree(mem2);
        fillfree(mem6);

        void * mem8 = fillalloc(128);
        CHECK(mem8);
        // merged block should start where mem6 previously started
        CHECK(mem8 == mem6);

        // by now it should be filled
        CHECK(a._used == a._size);
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
