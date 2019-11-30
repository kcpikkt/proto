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
