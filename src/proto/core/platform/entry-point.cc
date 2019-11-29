#include "proto/core/common.hh"
#include "proto/core/platform/common.hh"

namespace proto {
namespace platform {
    int _main(int argc, char ** argv);
    int runtime(int argc, char ** argv);
}
}

#if(PROTO_MAIN == 1)
#if defined(PROTO_PLATFORM_LINUX)
int main(int argc, char ** argv)
{
    return proto::platform::_main(argc, argv);
}
#elif defined(PROTO_PLATFORM_WINDOWS)
#include <windows.h>

int WINAPI WinMain(HINSTANCE instance_h, HINSTANCE prev_instance_h,
                   LPSTR lp_cmd_line, int n_show_cmd)
{
    using namespace proto::platform;
    runtime_state.instance_h = instance_h;
    runtime_state.prev_instance_h = prev_instance_h;
    return proto::platform::_main(__argc, __argv);
}
#endif
#endif

#include <stdio.h>
#include <assert.h>
// NOTE(kacper): Is having _main even useful?
int proto::platform::_main(int argc, char ** argv) {
    return proto::platform::runtime(argc, argv);
}
