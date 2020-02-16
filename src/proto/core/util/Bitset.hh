#pragma once
#include "proto/core/common.hh"
#include "proto/core/util/algo.hh"

#define _PROTO_BYTE_LSB(b) \
        (b & 0b00000001 ? 0 : b & 0b00000010 ? 1 : b & 0b00000100 ? 2 : b & 0b00001000 ? 3 : \
         b & 0b00010000 ? 4 : b & 0b00100000 ? 5 : b & 0b01000000 ? 6 : b & 0b10000000 ? 7 : 8)

#define _PROTO_BYTE_BITCOUNT(b) \
        (((b >> 0) & 1) + ((b >> 1) & 1) + ((b >> 2) & 1) + ((b >> 3) & 1) + \
         ((b >> 4) & 1) + ((b >> 5) & 1) + ((b >> 6) & 1) + ((b >> 7) & 1) )

namespace proto {

    template<u64 I>
    struct Bitset {
        static_assert(sizeof(u8) == 1);

        constexpr static auto size = (I + 7) / 8;
        constexpr static auto _bitsize = size * 8;
        constexpr static auto bitsize = I;
        u8 _arr[size] = {};


        inline void set(u64 index) {
            assert(index < I);
            _arr[index/8] |= (1 << (index % 8 ));
        }

        inline void unset(u64 index) {
            assert(index < I);
            _arr[index/8] &= ~(1 << (index % 8));
        }

        inline bool at(u64 index) const {
            assert(index < I);
            return (_arr[index/8] & (1 << (index % 8)));
        }

        inline void zero(){
            memset(_arr, '\0', size);
        }

        inline u64 lsb() {
            for(u64 i = 0; i < size; ++i)
                if(_arr[i]) return i*8 + _PROTO_BYTE_LSB(_arr[i]);

            return bitsize;
        }

        inline u64 bitcount() const {
            u64 acc = 0;
            for(u64 i = 0; i < size; ++i)
                if(_arr[i]) acc += _PROTO_BYTE_BITCOUNT(_arr[i]);

            return acc;
        }


        template<size_t _I>
        Bitset<I>& operator=(const Bitset<_I>& other) {
            for(u64 i=0; i<min(size, other.size); ++i)
                _arr[i] = other._arr[i];
            return *this;
        }

        template<size_t _I>
        Bitset<I>& operator|=(const Bitset<_I>& other) {
            for(u64 i=0; i<min(size, other.size); ++i)
                _arr[i] |= other._arr[i];
            return *this;
        }

        template<size_t _I>
        Bitset<I>& operator&=(const Bitset<_I>& other) {
            for(u64 i=0; i<min(size, other.size); ++i)
                _arr[i] &= other._arr[i];
            return *this;
        }

    };


    template<size_t I1, size_t I2,
             size_t I = max(I1, I2)> 
    Bitset<I> operator|(const Bitset<I1>& a, const Bitset<I2>& b) {
        Bitset<I> ret = a; ret |= b;
        return ret;
    }

    template<size_t I1, size_t I2,
             size_t I = max(I1, I2)> 
    Bitset<I> operator&(const Bitset<I1>& a, const Bitset<I2>& b) {
        Bitset<I> ret = a; ret &= b;
        return ret;
    }
} // namespace proto

