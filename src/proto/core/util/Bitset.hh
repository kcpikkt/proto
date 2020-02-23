#pragma once
#include "proto/core/common.hh"
#include "proto/core/util/algo.hh"

#define _PROTO_BYTE_LSB(B) \
        ((B) & 0b00000001 ? 0 : (B) & 0b00000010 ? 1 : (B) & 0b00000100 ? 2 : (B) & 0b00001000 ? 3 : \
         (B) & 0b00010000 ? 4 : (B) & 0b00100000 ? 5 : (B) & 0b01000000 ? 6 : (B) & 0b10000000 ? 7 : 8)

#define _PROTO_BYTE_BITCOUNT(B) \
        ((((B) >> 0) & 1) + (((B) >> 1) & 1) + (((B) >> 2) & 1) + (((B) >> 3) & 1) + \
         (((B) >> 4) & 1) + (((B) >> 5) & 1) + (((B) >> 6) & 1) + (((B) >> 7) & 1) )

#define _PROTO_BYTE_LOMASK(N) ((u8)((u16)(1 << (N)) - 1))
#define _PROTO_BYTE_HIMASK(N) ((u8)(~(_PROTO_BYTE_LOMASK(8 - (N)))))

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

        inline void set_range(u64 begin, u64 end) {
            assert(begin < end);
            assert(begin < bitsize);
            assert(end <= bitsize);

            //TODO(kacper): this is lazy code, make it proper
            for(u64 i=begin; i<end; i++) set(i);
        }

        inline void toggle(u64 index) {
            assert(index < I);
            _arr[index/8] ^= (1 << (index % 8 ));
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

        inline u64 bitcount_range(u64 begin, u64 end) {
            assert(begin < end);
            assert(begin < bitsize);
            assert(end <= bitsize);
            u64 acc = 0;

            u8 bmask = _PROTO_BYTE_HIMASK( 8 - (begin % 8) );
            u8 emask = _PROTO_BYTE_LOMASK( end % 8 );

            u64 bbyte = begin/8, ebyte = end/8;
        
            if(bbyte == ebyte) {
                acc = _PROTO_BYTE_BITCOUNT( _arr[ bbyte ] & bmask & emask );
                                            
            } else {
                u64 i = bbyte;
                // FUN BUG: there was i++ in macro and it expaned 8 times like that
                acc += _PROTO_BYTE_BITCOUNT( _arr[i] & bmask ); 

                for(++i ; i < ebyte; ++i)
                    if(_arr[i]) acc += _PROTO_BYTE_BITCOUNT(_arr[i]);

                acc += _PROTO_BYTE_BITCOUNT( _arr[i] & emask ); 
            }

            assert(acc <= (end - begin));

            return acc;
        }

        //inline u64 bitcount_until(u64 index) const {
        //    assert(index <= bitsize);
        //    u64 acc = 0;
        //    for(u64 i = 0; i < index/8; ++i)
        //        if(_arr[i]) acc += _PROTO_BYTE_BITCOUNT(_arr[i]);

        //    acc += _PROTO_BYTE_BITCOUNT(_arr[index/8] & _PROTO_BYTE_MASK_LEFT(index%8));

        //    return acc;
        //}

        inline bool is_zero() {
            for(u64 i = 0; i < size; ++i)
                if(_arr[i]) return false;

            return true;
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



    template<size_t I1, size_t I2> 
    inline bool operator!=(const Bitset<I1>& a, const Bitset<I2>& b) {
        if constexpr (I1 != I2) {
            return true;
        } else {
            for(u64 i=0; i<a.size; i++)
                if(a._arr[i] != b._arr[i]) return true;

            return false;
        }
    }

    template<size_t I1, size_t I2> 
    inline bool operator==(const Bitset<I1>& a, const Bitset<I2>& b) {
        return !(a != b);
    }

    template<size_t I1, size_t I2,
             size_t I = max(I1, I2)> 
    Bitset<I> operator&(const Bitset<I1>& a, const Bitset<I2>& b) {
        Bitset<I> ret = a; ret &= b;
        return ret;
    }

    template<size_t I1, size_t I2,
             size_t I = max(I1, I2)> 
    Bitset<I> operator|(const Bitset<I1>& a, const Bitset<I2>& b) {
        Bitset<I> ret = a; ret |= b;
        return ret;
    }


} // namespace proto

