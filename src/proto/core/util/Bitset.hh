#pragma once
#include "proto/core/common.hh"
#include "proto/core/common.hh"

#define _PROTO_BYTE_LSB(b) \
        (b & 0b00000001 ? 0 : b & 0b00000010 ? 1 : b & 0b00000100 ? 2 : b & 0b00001000 ? 3 : \
         b & 0b00010000 ? 4 : b & 0b00100000 ? 5 : b & 0b01000000 ? 6 : b & 0b10000000 ? 7 : 8)

namespace proto {

    template<size_t I>
    struct Bitset {
        using u8 = unsigned char;

        static_assert(sizeof(u8) == 1);

        constexpr static auto size = (I + 7) / 8;
        constexpr static auto bitsize = size * 8;
        u8 _arr[size] = {};


        inline void set(size_t index) {
            assert(index < I);
            _arr[index/8] |= (1 << (index % 8 ));
        }

        inline void unset(size_t index) {
            assert(index < I);
            _arr[index/8] &= ~(1 << (index % 8));
        }

        inline bool at(size_t index) {
            assert(index < I);
            return (_arr[index/8] & (1 << (index % 8)));
        }

        inline void zero(){
            memset(_arr, '\0', size);
        }

        inline size_t lsb() {
            for(size_t i = 0; i < size; ++i)
                if(i) return i*8 + _PROTO_BYTE_LSB(_arr[i]);

            return bitsize;
        }
    };
} // namespace proto

