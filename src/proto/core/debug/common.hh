#pragma once
#include "proto/core/common.hh"

namespace proto {
namespace debug {
    using Level = u8;
    using Category = u8;

    // TODO(kacper): bit silly maybe change that to enum
    // thank god for sed
    namespace level {
        constexpr static Level all       = 0;
        constexpr static Level info      = 1;
        constexpr static Level warning   = 2;
        constexpr static Level error     = 3;
        constexpr static Level assertion = 4;
    }

    namespace category {
        constexpr static Category all      = ~((Category)0);
        constexpr static Category main     = BIT(0);
        constexpr static Category io       = BIT(1);
        constexpr static Category memory   = BIT(2);
        constexpr static Category data     = BIT(3);
        constexpr static Category graphics = BIT(4);
        constexpr static Category serialization = BIT(5);
    }                                           

} // namespace proto {
} // namespace debug {

#define PROTO_NOOP ((void)0)

//TODO(kacper): expand __func__ inside?
//NOTE(kacper): meh, I get function name when assertion fails already
#define PROTO_NOT_IMPLEMENTED \
    { assert(0 && "Not implemented!"); }

#define PROTO_DEPRECATED \
    { debug_warn(proto::debug::category::main, \
                 "Usage of deprecated functionality. ", __PRETTY_FUNCTION__); }


