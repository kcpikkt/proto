#pragma once
#include "proto/core/debug/markers.hh"

#define proto_assert(COND) assert(COND);

#define proto_assert_marker(COND, MARKER)                   \
    static_assert(meta::is_same_v<decltype(COND), bool>);   \
    if(!(COND)) {                                           \
        log_debug_marker(proto::debug::category::main, MARKER); \
        proto_assert(COND);                                 \
    }
