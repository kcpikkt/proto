#pragma once
#include "proto/core/common.hh"
#include "proto/core/debug/common.hh"
#include "proto/core/io.hh"
#include "proto/stacktrace.hh"
#include <assert.h>

#define LOGGING_NO_COLOR
#ifdef LOGGING_NO_COLOR
    #define RED(STR) STR
    #define YELLOW(STR) STR
    #define CYAN(STR) STR
    #define PURPLE(STR) STR
#else
    #define COL_NC "\e[0m"
    #define COL_RED "\e[1;31m"
    #define COL_YELLOW "\e[1;33m"
    #define COL_CYAN "\e[0;36m"
    #define COL_PURPLE "\e[0;35m"
    #define RED(STR) COL_RED STR COL_NC
    #define YELLOW(STR) COL_YELLOW STR COL_NC
    #define CYAN(STR) COL_CYAN STR COL_NC
    #define PURPLE(STR) COL_PURPLE STR COL_NC
#endif



namespace proto {
namespace debug {
    struct logging {
        static void _log_header(const Level level,
                                [[maybe_unused]]const Category log_category,
                                [[maybe_unused]]const char * file,
                                [[maybe_unused]]const uint64_t line)
        {
            if(level == level::info) {}
            else if (level == level::warning)
                printf(YELLOW("warning: "));
            else if (level == level::error)
                printf(RED("error: "));
            else if (level == level::assertion)
                printf("assertion: ");
            else
                assert(false && "unknown log level");
        }
        static void _debug_log_header(const Level level,
                                [[maybe_unused]]const Category log_category,
                                const char * file,
                                const uint64_t line)
        {
            printf("%s:%lu: ", file, line);
            if(level == level::info) {}
            else if (level == level::warning)
                printf(YELLOW("warning: "));
            else if (level == level::error)
                printf(RED("error: "));
            else
                assert(false && "unknown log level");
        }

 

        template<typename ...Ts>
        static void _log(const Level level,
                        [[maybe_unused]]const Category log_category,
                        const char * file,
                        const uint64_t line,
                        Ts... ts)
        {
            _log_header(level, log_category, file, line);
            proto::io::print(ts..., "\n");
            fflush(stdout);
        }
        template<typename ...Ts>
        static void _debug_log(const Level level,
                        [[maybe_unused]]const Category log_category,
                        const char * file,
                        const uint64_t line,
                        Ts... ts)
        {
            _debug_log_header(level, log_category, file, line);
            proto::io::print(ts..., "\n");
            fflush(stdout);
        }
    };
#define log_info(CATEGORY, ...) (         \
    proto::debug::logging::_log(          \
        proto::debug::level::info, CATEGORY,     \
        __FILE__, __LINE__, __VA_ARGS__))

#define log_warn(CATEGORY, ...) (         \
    proto::debug::logging::_log(          \
        proto::debug::level::warning, CATEGORY,  \
        __FILE__, __LINE__, __VA_ARGS__))

#define log_error(CATEGORY, ...) (        \
    proto::debug::logging::_log(          \
        proto::debug::level::error, CATEGORY,    \
        __FILE__, __LINE__, __VA_ARGS__))


#define log_info_f(CATEGORY, fmt, ...) (  \
    proto::debug::logging::_log_f(        \
        proto::debug::level::info, CATEGORY,     \
        __FILE__, __LINE__, fmt, __VA_ARGS__))

#define log_warn_f(CATEGORY, fmt, ...) (  \
    proto::debug::logging::_log_f(             \
        proto::debug::level::warning, CATEGORY,        \
        __FILE__, __LINE__, fmt, __VA_ARGS__))

#define log_error_f(CATEGORY, fmt, ...) (       \
    proto::debug::logging::_log_f(             \
        proto::debug::level::error, CATEGORY,          \
        __FILE__, __LINE__, fmt, __VA_ARGS__))


#define debug_info(CATEGORY, ...) (        \
    proto::debug::logging::_debug_log(          \
        proto::debug::level::info, CATEGORY,      \
        __FILE__, __LINE__, __VA_ARGS__))


#define debug_warn(CATEGORY, ...) (        \
    proto::debug::logging::_debug_log(          \
        proto::debug::level::warning, CATEGORY,   \
        __FILE__, __LINE__, __VA_ARGS__))

#define debug_error(CATEGORY, ...) (       \
    proto::debug::logging::_debug_log(          \
        proto::debug::level::error, CATEGORY,     \
        __FILE__, __LINE__, __VA_ARGS__))

#define cond_debug_info(COND, CATEGORY, ...) {                 \
    static_assert(std::is_same<bool, decltype(COND)>::value); \
    if(COND) debug_info(CATEGORY, __VA_ARGS__);}

#define cond_debug_warn(COND, CATEGORY, ...) {                 \
    static_assert(std::is_same<bool, decltype(COND)>::value); \
    if(COND) debug_warn(CATEGORY, __VA_ARGS__);}

#define cond_debug_error(COND, CATEGORY, ...) {                \
    static_assert(std::is_same<bool, decltype(COND)>::value); \
    if(COND) debug_error(CATEGORY, __VA_ARGS__);}

// TODO(kacper): change log level to assertion
#define assert_info(COND, CATEGORY, ...) {         \
    if(!(COND)) {                                 \
        trace();                                  \
        proto::debug::logging::_debug_log(        \
            proto::debug::level::error, CATEGORY,     \
            __FILE__, __LINE__, __VA_ARGS__);     \
        assert(COND);                             \
    }}

#define vardump(VARNAME) (           \
    proto::debug::logging::_log(          \
        proto::debug::level::info, proto::debug::category::main, \
        __FILE__, __LINE__, PROTO_STR(VARNAME), "=", (VARNAME) ))

} //namespace debug
} //namespace proto

