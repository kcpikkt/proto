#pragma once
#include "proto/core/common.hh"
// bit twiddling general

namespace proto {
    constexpr static u16 u8_lomask[] =
        { 0,
          u8((1 <<  1) - 1),
          u8((1 <<  2) - 1),
          u8((1 <<  3) - 1),
          u8((1 <<  4) - 1),
          u8((1 <<  5) - 1),
          u8((1 <<  6) - 1),
          u8((1 <<  7) - 1),
          u8(~0),
    };

    constexpr static u8 u8_himask[] =
        { 0,
          u8(~((1 <<  7) - 1) ),
          u8(~((1 <<  6) - 1) ),
          u8(~((1 <<  5) - 1) ),
          u8(~((1 <<  4) - 1) ),
          u8(~((1 <<  3) - 1) ),
          u8(~((1 <<  2) - 1) ),
          u8(~((1 <<  1) - 1) ),
          u8(~0),
    };

    constexpr static u16 u16_lomask[] =
        { 0,
          u16((1 <<  1) - 1),
          u16((1 <<  2) - 1),
          u16((1 <<  3) - 1),
          u16((1 <<  4) - 1),
          u16((1 <<  5) - 1),
          u16((1 <<  6) - 1),
          u16((1 <<  7) - 1),
          u16((1 <<  8) - 1),
          u16((1 <<  9) - 1),
          u16((1 << 10) - 1),
          u16((1 << 11) - 1),
          u16((1 << 12) - 1),
          u16((1 << 13) - 1),
          u16((1 << 14) - 1),
          u16((1 << 15) - 1),
          u16( ~0),
    };

    constexpr static u16 u16_himask[] =
        { 0,
          u16(~((1 << 15) - 1) ),
          u16(~((1 << 14) - 1) ),
          u16(~((1 << 13) - 1) ),
          u16(~((1 << 12) - 1) ),
          u16(~((1 << 11) - 1) ),
          u16(~((1 << 10) - 1) ),
          u16(~((1 <<  9) - 1) ),
          u16(~((1 <<  8) - 1) ),
          u16(~((1 <<  7) - 1) ),
          u16(~((1 <<  6) - 1) ),
          u16(~((1 <<  5) - 1) ),
          u16(~((1 <<  4) - 1) ),
          u16(~((1 <<  3) - 1) ),
          u16(~((1 <<  2) - 1) ),
          u16(~((1 <<  1) - 1) ),
          u16(~0),
    };

};