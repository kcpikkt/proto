#pragma once
#include "proto/core/debug/logging.hh"
#include "proto/core/common/types.hh"
namespace proto { 
namespace debug { 

static void hexdump(void * buffer, s32 lines) {
    auto is_printable = [](char c) {return (c >= 21 && c <= 126);};

    if(lines > 0) {
        for(s32 j=0; j<lines; ++j) {
            printf("+%08x: ", j * 16);

            for(int i=0; i<8; ++i)
                printf(" %04x", *((u16*)buffer + j * 8 + i));

            printf("  ");
            for(int i=0; i<16; ++i) {
                char c = *((u8*)buffer + j * 16 + i);
                printf("%c", is_printable(c) ? c : '.');
            }

            printf("\n");
        }
    } else {
        for(s32 j=0; j>lines; --j) {
            printf("-%08x:", -j * 16);

            for(int i=0; i<8; ++i)
                printf(" %04x", *((u16*)buffer + j * 8 + i));

            printf("\n");
        }
    }
}

} // namespace debug 
} // namespace proto 
