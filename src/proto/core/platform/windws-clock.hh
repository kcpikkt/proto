#pragma once
#include "proto/core/platform/macros.hh"
#if defined(PROTO_PLATFORM_WINDOWS)
#include "proto/core/platform/clock.hh"

namespace proto {
namespace platform {

struct WindowsClockImpl : ClockCRTPInterface<WindowsClockImpl> {
    void impl();
    void tick();
};

}
}

#else
#error windows specific header included in non windows platform
#endif
