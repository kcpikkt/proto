#pragma once
#include "proto/core/common.hh"
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
    using log_category_t = uint64_t;
    using log_level_t = uint8_t;

    constexpr log_level_t info      = 1;
    constexpr log_level_t warning   = 2;
    constexpr log_level_t error     = 3;
    constexpr log_level_t assertion = 4;

    namespace category {
        constexpr log_category_t main     = BIT(1);
        constexpr log_category_t io       = BIT(2);
        constexpr log_category_t memory   = BIT(3);
        constexpr log_category_t data     = BIT(4);
        constexpr log_category_t graphics = BIT(5);
        constexpr log_category_t physics  = BIT(6);
    }

    struct logging {
        static void _log_header(const log_level_t level,
                                [[maybe_unused]]const log_category_t log_category,
                                [[maybe_unused]]const char * file,
                                [[maybe_unused]]const uint64_t line)
        {
            if(level == info) {}
            else if (level == warning)
                printf(YELLOW("warning: "));
            else if (level == error)
                printf(RED("error: "));
            else if (level == assertion)
                printf("assertion: ");
            else
                assert(false && "unknown log level");
        }
        static void _debug_log_header(const log_level_t level,
                                [[maybe_unused]]const log_category_t log_category,
                                const char * file,
                                const uint64_t line)
        {
            printf("%s:%lu: ", file, line);
            if(level == info) {}
            else if (level == warning)
                printf(YELLOW("warning: "));
            else if (level == error)
                printf(RED("error: "));
            else
                assert(false && "unknown log level");
        }

 

        template<typename ...Ts>
        static void _log(const log_level_t level,
                        [[maybe_unused]]const log_category_t log_category,
                        const char * file,
                        const uint64_t line,
                        Ts... ts)
        {
            _log_header(level, log_category, file, line);
            proto::io::print(ts..., "\n");
            fflush(stdout);
        }
        template<typename ...Ts>
        static void _debug_log(const log_level_t level,
                        [[maybe_unused]]const log_category_t log_category,
                        const char * file,
                        const uint64_t line,
                        Ts... ts)
        {
            _debug_log_header(level, log_category, file, line);
            proto::io::print(ts..., "\n");
            fflush(stdout);
        }
    };
#define log_info(CATEGORY, ...) (           \
    proto::debug::logging::_log(          \
        proto::debug::info, CATEGORY,      \
        __FILE__, __LINE__, __VA_ARGS__))

#define log_warn(CATEGORY, ...) (           \
    proto::debug::logging::_log(          \
        proto::debug::warning, CATEGORY,   \
        __FILE__, __LINE__, __VA_ARGS__))

#define log_error(CATEGORY, ...) (          \
    proto::debug::logging::_log(          \
        proto::debug::error, CATEGORY,     \
        __FILE__, __LINE__, __VA_ARGS__))


#define log_info_f(CATEGORY, fmt, ...) (           \
    proto::debug::logging::_log_f(          \
        proto::debug::info, CATEGORY,      \
        __FILE__, __LINE__, fmt, __VA_ARGS__))

#define log_warn_f(CATEGORY, fmt, ...) (        \
    proto::debug::logging::_log_f(             \
        proto::debug::warning, CATEGORY,        \
        __FILE__, __LINE__, fmt, __VA_ARGS__))

#define log_error_f(CATEGORY, fmt, ...) (       \
    proto::debug::logging::_log_f(             \
        proto::debug::error, CATEGORY,          \
        __FILE__, __LINE__, fmt, __VA_ARGS__))


#define debug_info(CATEGORY, ...) (        \
    proto::debug::logging::_debug_log(          \
        proto::debug::info, CATEGORY,      \
        __FILE__, __LINE__, __VA_ARGS__))


#define debug_warn(CATEGORY, ...) (        \
    proto::debug::logging::_debug_log(          \
        proto::debug::warning, CATEGORY,   \
        __FILE__, __LINE__, __VA_ARGS__))

#define debug_error(CATEGORY, ...) (       \
    proto::debug::logging::_debug_log(          \
        proto::debug::error, CATEGORY,     \
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
            proto::debug::error, CATEGORY,     \
            __FILE__, __LINE__, __VA_ARGS__);     \
        assert(COND);                             \
    }}

#define vardump(VARNAME) (           \
    proto::debug::logging::_log(          \
        proto::debug::info, proto::debug::category::main, \
        __FILE__, __LINE__, PROTO_STR(VARNAME), "=", (VARNAME) ))



    // NOTE(kacper): Idea of Marker is (as far as I know) the
    //               same as Vulkan's Debug Markers.
    //               This solution requires type of given object,
    //               that we want to give debug name to, to inherit
    //               Marker, so that it can store pointers to its debug info.
    //               Now, of course, this is not the most flexible solution.
    //               Would be nice if we could just assign debug marker to
    //               any instance of any class we want.
    //               This would require keeping a map of debug info
    //               and some kind of appropriate object identifier as key.
    //               Its clear that addresses are not the best candidate
    //               for identifiers, though for case, when we assign debug info
    //               to some object and we are sure that it wont change its location
    //               there is some potential for usability of such VolatileMarker
    //               
    //               Object that are assets we can identify with their AssetHandle
    //               of which copy they should store, thus it should be easy to
    //               get this behaviour just for Asset Objects.
    //
    // TODO(kacper): AssetMarker
    // MAYBE TODO(kacper): VolatileMarker
    
    struct Marker {
        const char * debug_name = nullptr;
        const char * debug_info = nullptr;
    };
    // for non debug versions, just to not inheritance break syntax
    struct EmptyMarker {}; 

#if defined(PROTO_DEBUG)
# define PROTO_DEBUG_MARKER public proto::debug::Marker
#else
# define PROTO_DEBUG_MARKER private proto::debug::EmptyMarker
#endif

#define set_debug_marker_variadic(_1, _2, _3, NAME, ...) NAME

#define set_debug_marker(...)                                         \
    set_debug_marker_variadic(__VA_ARGS__,                            \
                              set_debug_marker_full,                  \
                              set_debug_marker_partial)(__VA_ARGS__)

#define set_debug_marker_partial(OBJECT, NAME) {                      \
    static_assert(proto::meta::is_class_v<decltype(OBJECT)>);         \
    static_assert(proto::meta::is_base_of_v<proto::debug::Marker,     \
                  decltype(OBJECT)>,                                  \
                  "object type does not inherit DebugMarker");        \
    char * debug_name_mem = (char*)                                   \
    proto::context->gp_debug_strings_allocator.alloc(sizeof(NAME));   \
    if(debug_name_mem){                                               \
        strncpy(debug_name_mem, NAME, sizeof(NAME));                  \
        (OBJECT).debug_name = debug_name_mem;                         \
    }}

#define set_debug_marker_full(OBJECT, NAME, INFO) {                   \
    set_debug_marker_partial(OBJECT, NAME)                            \
    char * debug_info_mem = (char*)                                   \
    proto::context->gp_debug_strings_allocator.alloc(sizeof(INFO));   \
    if(debug_info_mem){                                               \
        strncpy(debug_info_mem, INFO, sizeof(INFO));                  \
        (OBJECT).debug_info = debug_info_mem;                         \
    }}

#define log_debug_marker(CATEGORY, OBJECT) {                         \
    static_assert(proto::meta::is_class_v<decltype(OBJECT)>);        \
    static_assert(proto::meta::is_base_of_v<proto::debug::Marker,    \
                  decltype(OBJECT)>,                                 \
                  "object type does not inherit DebugMarker");       \
                                                                     \
    log_info((CATEGORY),                                             \
             (OBJECT).debug_name != nullptr                          \
                ? (OBJECT.debug_name)                                \
                : "(noname)");                                       \
                                                                     \
    if((OBJECT).debug_info != nullptr) {                             \
        log_info((CATEGORY),                                         \
                 (OBJECT.debug_info));                               \
    }}

} //namespace debug
} //namespace proto

