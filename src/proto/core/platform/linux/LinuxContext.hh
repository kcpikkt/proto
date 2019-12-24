#pragma once
#include "proto/core/platform/macros.hh"

#include <X11/X.h>
#include <X11/Xlib.h>

namespace proto {
namespace platform {

struct LinuxContext {
    Display * display;
    Window window;

    //int inotify_fd = -1;
    //Array<int> watched_files;
};

}
}
