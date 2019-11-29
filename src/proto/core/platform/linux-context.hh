#pragma once
#include "proto/core/platform/macros.hh"
#if defined(PROTO_PLATFORM_LINUX)

#include <X11/X.h>
#include <X11/Xlib.h>

namespace proto {
namespace platform {

struct LinuxContext {
    Display * display;
    Window window;
};

}
}

#else
#error linux specific header included in non linux platform
#endif
