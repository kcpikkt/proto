#include "proto/core/platform/common.hh"
#if defined(PROTO_PLATFORM_WINDOWS)
#include "proto/core/platform/api.hh"
#include "proto/core/platform/windows-runtime.hh"
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <windows.h>

int proto::platform::print_f(const char * fmt, ...) {
    static char buffer[1024];
    va_list args;
    va_start(args, fmt);
    int buflen = ::vsnprintf( buffer, 1024, fmt, args );
    va_end(args);

    unsigned long len = 0;
    ::WriteFile(runtime_state.stdout_h, (void*)buffer, buflen, &len, NULL);

    return 0;
}
#endif
