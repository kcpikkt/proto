#pragma once
#include "proto/core/debug/logging.hh"
#include "proto/core/common/types.hh"
namespace proto { 
namespace debug { 

void hexdump(void * buffer, u32 lines) {
    for(s32 j=0; j<lines; ++j) {
        printf("+%08x:", j * 16);
        for(int i=0; i<8; ++i) {
            u16 hex = *((u16*)buffer + j * 8 + i);
            printf(" %04x", hex);
        }
        printf("\n");
    }
}


} // namespace debug 
} // namespace proto 
