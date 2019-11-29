#pragma once
#include "proto/core/platform/macros.hh"
#if defined(PROTO_PLATFORM_WINDOWS)
namespace proto {
namespace platform {

struct WindowsContext {
    // HWND window_h;
    // HINSTANCE instance_h;
    // HINSTANCE prev_instance_h;
    // HANDLE stdin_h;
    // HANDLE stdout_h;
    // HANDLE stderr_h;
};

}
}

#else
#error windows specific header included in non windows platform
#endif
