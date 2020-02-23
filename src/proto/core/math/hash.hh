#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/util/StringView.hh"
namespace proto {
namespace hash {
//    #define PROTO_CRC32_MAX_INPUT_LEN 256
//    //    constexpr static u32 crc32_seed = 0xFFFFFFFF;
//
constexpr static u32 crc32_poly = 0xEDB88320;
static constexpr u32 crc32(StringView data, u32 prev_crc = 0) {
    u32 crc = ~prev_crc;

    u32 length = data.length;
    assert(data.length);

    // It was cast to unsigned but cant do that in constexpr
    // shouldnt be a problem as long as integers are twos compliment
    const char * current = data.str;

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
