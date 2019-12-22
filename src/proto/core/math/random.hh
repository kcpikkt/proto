#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/debug/logging.hh"
namespace proto {
namespace random {



constexpr static u64 mt64_tab_len = 312;
static u64 mt64_tab[mt64_tab_len];
static u64 mt64_index = mt64_tab_len + 1;

constexpr static u64 lo32b = ((u64)1 << 32) - 1;
constexpr static u64 hi32b = ((u64)1 << 32) - 1;

void seed_mt64(u64 seed) {
    mt64_tab[0] = seed;
    mt64_index = mt64_tab_len;

    u64 prev = mt64_tab[0];
    for(u64 i=1; i<mt64_tab_len; i++) {
        mt64_tab[i] =
            (1812433253ull * (prev xor (prev >> 62)) + i) & lo32b;
        prev = mt64_tab[i];
    }
}

static void _twist_mt64() {
    for(u64 i=0; i <mt64_tab_len; i++) {
        u64 x =
            (mt64_tab[i] & hi32b) +
            (mt64_tab[(i + 1) % mt64_tab_len] & lo32b);
        u64 xa = x >> 1;
        if((x % 2) != 0)
            xa = xa xor 0xB5026F5AA96619E9ull;

        mt64_tab[i] = mt64_tab[(i + 156) % mt64_tab_len] xor xa;
    }
    mt64_index = 0;
}

u64 next_mt64() {
    if(mt64_index >= mt64_tab_len) {
        assert(mt64_index == mt64_tab_len && "mt64 was not seeded");
        _twist_mt64();
    }
    u64 ret = mt64_tab[mt64_index++];
    ret = ret xor ((ret >> 29) & 0x5555555555555555ull);
    ret = ret xor ((ret << 17) & 0x71D67FFFEDA60000ull);
    ret = ret xor ((ret << 37) & 0xFFF7EEE000000000ull);
    ret = ret xor  (ret >> 43);

    return ret;
}

float randf01() {
    return ((float)next_mt64()) / ((float)(~0ull));
}


} // namespace random
} // namespace proto
