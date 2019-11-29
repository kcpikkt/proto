#pragma once

#if defined(_WIN32)
    #define PROTO_PLATFORM_WINDOWS
    #define WIN32_LEAN_AND_MEAN

    #if defined(_WIN64)
        #define PROTO_PLATFORM_WIN64
    #else
        #define PROTO_PLATFORM_WIN32
    #endif
//    #error FIXME: windows support

#elif defined(__linux__)
    #define PROTO_PLATFORM_LINUX

#elif defined (__APPLE__)
    #define PROTO_PLATFORM_MAC
    #error FIXME: macosx support

#elif
    #error unsupported OS
#endif

#if !defined(PROTO_MAIN)
#define PROTO_MAIN 1
#endif

// NOTE(kacper): well, well, in future it may support not only OPENGL so
#define PROTO_OPENGL
