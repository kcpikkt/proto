#pragma once
#include "proto/core/math/hash.hh"
namespace proto {
namespace hash {
//    #define PROTO_CRC32_MAX_INPUT_LEN 256
//    //    constexpr static u32 crc32_seed = 0xFFFFFFFF;
//
    constexpr static u32 crc32_poly = 0xEDB88320;
    static u32 crc32(StringView data, u32 prev_crc = 0) {
        u32 crc = ~prev_crc;

        u32 length = data.length();
        assert(data.length());

        u8* current = (u8*)data.str();

        while(length--){ 
            crc ^= *(current++);

            for(u8 i=0; i < 8; i++) {
                //if(crc & 1)
                //    crc = (crc >> 1) ^ crc32_poly;
                //else
                //    crc = (crc >> 1);
                crc = (crc >> 1) ^ (crc & 1) * crc32_poly;
            }
        }
        return ~crc;
    }
} // namespace hash
} // namespace proto
