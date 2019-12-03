#pragma once
#include "proto/core/common.hh"

namespace proto {
namespace debug {
    using log_category_t = u64;

    namespace category {
        constexpr log_category_t main     = BIT(1);
        constexpr log_category_t io       = BIT(2);
        constexpr log_category_t memory   = BIT(3);
        constexpr log_category_t data     = BIT(4);
        constexpr log_category_t graphics = BIT(5);
        constexpr log_category_t physics  = BIT(6);
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
                 "Usage of deprecated function. ", __PRETTY_FUNCTION__); }


