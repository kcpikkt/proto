#pragma once
#include "proto/core/debug/logging.hh"

//TODO(kacper): expand __func__ inside?
//NOTE(kacper): meh, I get function name when assertion fails already
#define PROTO_NOT_IMPLEMENTED \
    { assert(0 && "Not implemented!"); }

// TODO(kacper): would be cool to get a caller from trace
#define PROTO_DEPRECATED \
    { debug_warn(proto::debug::category::main, \
                 "Usage of deprecated function."); }


