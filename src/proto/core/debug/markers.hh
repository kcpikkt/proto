#pragma once

namespace proto {
namespace debug {
     // NOTE(kacper): Idea of Marker is (as far as I am concerned) the
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
    // for non debug versions, just to avoid breaking inheritance syntax
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
                  "object type does not inherit debug::Marker");      \
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
                  "object type does not inherit debug::Marker");     \
                                                                     \
    log_info((CATEGORY), "marker: ",                                 \
             (OBJECT).debug_name != nullptr                          \
                ? (OBJECT.debug_name)                                \
                : "(noname)");                                       \
                                                                     \
    if((OBJECT).debug_info != nullptr) {                             \
        log_info((CATEGORY),                                         \
                 (OBJECT.debug_info));                               \
    }}


}
}
