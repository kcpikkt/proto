#pragma once

#include "proto/core/common/types.hh"


#if defined(NDEBUG)
# define PROTO_RELEASE
#else
# if !defined(PROTO_DEBUG)
#  define PROTO_DEBUG
# endif
#endif

#define PROTO_STR_HELPER(X) #X
#define PROTO_STR(X) PROTO_STR_HELPER(X)

#define BIT(N) ((proto::u64)1 << N)
