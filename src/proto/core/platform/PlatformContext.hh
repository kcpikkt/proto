#pragma once
#include "proto/core/platform/macros.hh"

#if defined(PROTO_PLATFORM_LINUX)
# include "proto/core/platform/linux/LinuxContext.hh"
#elif defined(PROTO_PLATFORM_WINDOWS)
# error not implemented
#elif defined(PROTO_PLATFORM_MAC)
# error not implemented
#endif

namespace proto {
namespace platform {

struct PlatformContext :
#if defined(PROTO_PLATFORM_LINUX)
    LinuxContext
#elif defined(PROTO_PLATFORM_WINDOWS)
# error not implemented
#elif defined(PROTO_PLATFORM_MAC)
# error not implemented
#endif
{};

} // namespace platform 
} // namespace proto
