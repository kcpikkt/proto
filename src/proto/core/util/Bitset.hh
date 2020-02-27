#pragma once
#include "proto/core/common.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/util/algo.hh"
#include "proto/core/debug.hh"
#include "proto/core/util/bits.hh"


#define _PROTO_BYTE_LSB(B) \
        ((B) & 0b00000001 ? 0 : (B) & 0b00000010 ? 1 : (B) & 0b00000100 ? 2 : (B) & 0b00001000 ? 3 : \
         (B) & 0b00010000 ? 4 : (B) & 0b00100000 ? 5 : (B) & 0b01000000 ? 6 : (B) & 0b10000000 ? 7 : 8)

#define _PROTO_BYTE_BITCOUNT(B) \
        ((((B) >> 0) & 1) + (((B) >> 1) & 1) + (((B) >> 2) & 1) + (((B) >> 3) & 1) + \
         (((B) >> 4) & 1) + (((B) >> 5) & 1) + (((B) >> 6) & 1) + (((B) >> 7) & 1) )

namespace proto {
    // 0 means not compile time size in case bitset is placed.
    template<u64 I> 
    struct _Bitset {
        constexpr static u64 arrsize = mem::align(I, 8) / 8;
        constexpr static u64 bitsize = I;
        u8 _arr[arrsize] = {};
    };

    template<>
    struct _Bitset<0> {
        u64 arrsize = 0;
        u64 bitsize = 0;
        u8 * _arr = nullptr;

        inline void place(void * ptr, u64 bitsize) {
            assert(ptr);
            assert(bitsize);

            this->arrsize = mem::align(bitsize, 8) / 8;
            this->bitsize = bitsize;

            _arr = ((u8*)ptr);
        }
    };

    template<u64 I = 0> 
    struct Bitset : public _Bitset<I> {
        static_assert(sizeof(u8) == 1);
        using M = _Bitset<I>;

        inline Bitset<I>& set(u64 index) {
            assert(index < M::bitsize);
            M::_arr[index/8] |= (1 << (index % 8 ));
            return *this;
        }

        inline Bitset<I>& set_range(u64 begin, u64 end) {
            assert(begin < end);
            assert(begin < M::bitsize);
            assert(end <= M::bitsize);

            //TODO(kacper): this is lazy code, make it proper
            for(u64 i=begin; i<end; i++) set(i);
            return *this;
        }

        inline Bitset<I>& set_range(Range<u64> r) {
            return set_range(r.begin, r.end);
        }

        template<bool bit> inline u64 find_span(u64 bitw) {
            // a bit silly to look for 0 span but ok.
            if(bitw == 0)          return 0;
            if(bitw == 1)          return lsb<bit>();
            if(bitw == M::bitsize) return are_all_bits<bit>() ? 0 : M::bitsize;
            if(bitw > M::bitsize)  return M::bitsize;
            if(bitw < 16) {
                u16 word, mask;
                for(u64 i=0; i < M::arrsize - 1; ++i) {
                    if constexpr (bit) {
                        if( M::_arr[i] == 0 &&  M::_arr[i + 1] == 0) continue; }
                    else {
                        if(~M::_arr[i] == 0 && ~M::_arr[i + 1] == 0) continue; }

                    for(s8 wnd = 0; wnd + bitw <= 16; ++wnd) {
                        mask = u16_lomask[ bitw + wnd ] & ~u16_lomask[wnd];

                        if constexpr (bit) { word =  (*((u16*)(M::_arr + i))); }
                        else               { word = ~(*((u16*)(M::_arr + i))); }

                        if( (word & mask) == mask) return i * 8 + wnd;
                    }
                }
                return M::bitsize;
            }

            // For fast rejection we ask how many consecutive 0 BYTES
            // we need in a row for span be even possible, this value is k.
            // it makes sense when you think about it.
            // For example: there is no way you can put 15 zero bits without
            // one whole byte being zero.
            // NOTE(kacper): min for bitw = 2-7 case
            u64 k = max(1, bitw/8) - 1;

            if((k + 1) > M::arrsize) return M::bitsize;

            u64 cnt = 0; // count of zero bytes so far
            u64 idx = 0; // index from which we are counting

            // we are looking for k consecutive all one/zero bytes
            // ...|     |     | OK  | OK  | OK  |     |...
            // ...|(n+0)|(n+1)|(n+2)|(n+3)|(n+4)|(n+5)|...
            //                 ^-----^-----^
            // idx = (n+2), cnt = (n+4)-(n+2) = 2
            for(u64 i=1; i < M::arrsize - 1; ++i) {
                if(M::_arr[i] == (bit ? u8(~0) : u8(0)) ) {
                    if( !(cnt++) ) idx = i;
                } else {
                    cnt = 0; idx = i;
                }

                if(cnt == k) {
                    // example res bit window (here res is 9)
                    // |bbbbbbbb|...k...|bbbbbbbb|
                    //    ^----- ~~~~~~~ --^
                    // lobits = 6, hibits = 3
                    u8 lobyte_mask, hibyte_mask;

                    u8 lobyte = M::_arr[ idx - 1 ];
                    u8 hibyte = M::_arr[ i + 1   ];

                    if constexpr (!bit) {
                        lobyte = ~lobyte; hibyte = ~hibyte; }

                    u8 res = (bitw - k*8);

                    // since we allow for non 8 multiple bitsizes
                    u8 pad = (i + 1 == M::arrsize - 1)
                        ? (M::arrsize * 8 - M::bitsize) : 0;

                    s8 lobits = min(8, res);
                    s8 hibits = res - lobits;

                    for(; lobits > 0 && hibits < 8 - pad; --lobits, ++hibits) {
                        lobyte_mask = u8_himask[ lobits ];
                        hibyte_mask = u8_lomask[ hibits ];

                        if( (lobyte & lobyte_mask) == lobyte_mask) {
                            if((hibyte & hibyte_mask) == hibyte_mask ) {
                                //FIXME
                                return idx * 8 - lobits;
                            } else {
                                // if we have no space at the hibyte
                                // there is no reason to continue
                                break;
                            }
                        }
                    }
                    // shift window by one
                    idx++;
                    if(cnt) cnt--;
                }
            }
            return M::bitsize;
        }

        inline u64 find_span1(u64 bitw) { return find_span<1>(bitw); }
        inline u64 find_span0(u64 bitw) { return find_span<0>(bitw); }

        inline void toggle(u64 index) {
            assert(index < M::bitsize);
            M::_arr[index/8] ^= (1 << (index % 8 ));
        }

        inline void unset(u64 index) {
            assert(index < M::bitsize);
            M::_arr[index/8] &= ~(1 << (index % 8));
        }

        inline bool at(u64 index) const {
            assert(index < M::bitsize);
            return (M::_arr[index/8] & (1 << (index % 8)));
        }

        inline bool at_and(u64 index) const
        { return at(index); }

        template<typename... Is>
        inline bool at_and(u64 i, Is... is) const {
            return (at(i) & at_and(is...));
        }

        inline bool at_or(u64 index) const
        { return at(index); }

        template<typename... Is>
        inline bool at_or(u64 i, Is... is) const {
            return (at(i) & at_or(is...));
        }


        inline void zerofill(){
            // FIXME(kacper):there are no arguments to ‘memset’ that depend on a
            // template parameter, so a declaration of ‘memset’ must be available
            // memset(_arr, '\0', arrsize);
            assert(0);
        }

        //  inline u64 find_range(){
        //  }

        template<bool bit = true> inline u64 lsb() const {
            for(u64 i = 0; i < M::arrsize; ++i) {
                if constexpr (bit) {
                    if( u8( M::_arr[i]) ) return i*8 + _PROTO_BYTE_LSB( u8( M::_arr[i]) );
                } else {
                    if( u8(~M::_arr[i]) ) return i*8 + _PROTO_BYTE_LSB( u8(~M::_arr[i]) );
                }
            }

            return M::bitsize;
        }

        inline u64 lsb1() const { return lsb<1>(); }
        inline u64 lsb0() const { return lsb<0>(); }

        template<bool bit> inline void fill() {
            for(u64 i = 0; i < M::arrsize; ++i) {
                if constexpr(bit) { M::_arr[i] = u8(~0); }
                else              { M::_arr[i] = u8( 0); }
            }
        }

        constexpr inline u8 _backpad_mask() const {
            u8 lastbytew = (8 - (M::arrsize * 8 - M::bitsize));
            return u8_lomask[ lastbytew ];
        }
        // you may be better of without conditional
        template<bool bit> inline u64 bitcount() const {
            u64 acc = 0;
            u8 byte;
            for(u64 i = 0; i < M::arrsize - 1; ++i) {
                if constexpr(bit) { byte =  M::_arr[i]; }
                else {              byte = ~M::_arr[i]; }

                if( byte ) acc += _PROTO_BYTE_BITCOUNT( byte );
            }
            if constexpr(bit) { byte = M::_arr[M::arrsize - 1] & _backpad_mask(); }
            else {              byte =~M::_arr[M::arrsize - 1] & _backpad_mask(); }

            acc += _PROTO_BYTE_BITCOUNT( byte );

            return acc;
        }

        inline u64 bitcount1() const { return bitcount<1>(); }
        inline u64 bitcount0() const { return bitcount<0>(); }

        inline u64 bitcount_range(u64 begin, u64 end) {
            assert(begin < end);
            assert(begin < M::bitsize);
            assert(end <= M::bitsize);
            u64 acc = 0;

            u8 bmask = u8_himask[ 8 - (begin % 8) ];
            u8 emask = u8_lomask[ end % 8 ];

            u64 bbyte = begin/8, ebyte = end/8;
        
            if(bbyte == ebyte) {
                acc = _PROTO_BYTE_BITCOUNT( M::_arr[ bbyte ] & bmask & emask );
                                            
            } else {
                u64 i = bbyte;
                // FUN BUG: there was i++ in macro and it expaned 8 times like that
                acc += _PROTO_BYTE_BITCOUNT( M::_arr[i] & bmask ); 

                for(++i ; i < ebyte; ++i)
                    if(M::_arr[i]) acc += _PROTO_BYTE_BITCOUNT(M::_arr[i]);

                acc += _PROTO_BYTE_BITCOUNT( M::_arr[i] & emask ); 
            }

            assert(acc <= (end - begin));

            return acc;
        }

        inline u64 bitcount_range(Range<u64> r) {
            return bitcount_range(r.begin, r.end);
        }

        template<bool bit> inline bool are_all_bits() {
            for(u64 i = 0; i < M::arrsize; ++i)
                if( M::_arr[i] != (bit ? u8(~0) : u8(0)) ) return false;

            return true;
        }

        inline bool is_zero() {
            return are_all_bits<0>();
        }

        template<u64 _I>
        Bitset<I>& operator=(const Bitset<_I>& other) {
            for(u64 i=0; i<min(M::arrsize, other.M::arrsize); ++i)
                M::_arr[i] = other.M::_arr[i];
            return *this;
        }

        template<u64 _I>
        Bitset<I>& operator|=(const Bitset<_I>& other) {
            for(u64 i=0; i<min(M::arrsize, other.M::arrsize); ++i)
                M::_arr[i] |= other.M::_arr[i];
            return *this;
        }

        template<u64 _I>
        Bitset<I>& operator&=(const Bitset<_I>& other) {
            for(u64 i=0; i<min(M::arrsize, other.arrsize); ++i)
                M::_arr[i] &= other.M::_arr[i];
            return *this;
        }

    };



    template<u64 I1, u64 I2> 
    inline bool operator!=(const Bitset<I1>& a, const Bitset<I2>& b) {
        if constexpr (I1 != I2) {
            return true;
        } else {
            for(u64 i=0; i<a.arrsize; i++)
                if(a._arr[i] != b._arr[i]) return true;

            return false;
        }
    }

    template<u64 I1, u64 I2> 
    inline bool operator==(const Bitset<I1>& a, const Bitset<I2>& b) {
        return !(a != b);
    }

    template<u64 I1, u64 I2,
             u64 I = max(I1, I2)> 
    Bitset<I> operator&(const Bitset<I1>& a, const Bitset<I2>& b) {
        Bitset<I> ret = a; ret &= b;
        return ret;
    }

    template<u64 I1, u64 I2,
             u64 I = max(I1, I2)> 
    Bitset<I> operator|(const Bitset<I1>& a, const Bitset<I2>& b) {
        Bitset<I> ret = a; ret |= b;
        return ret;
    }


} // namespace proto

