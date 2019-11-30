#pragma once
#include "proto/core/meta.hh"
#include "proto/core/util/algo.hh"
#include "proto/core/platform/api.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/string.hh"
#include <stdio.h>
#include <tgmath.h>
#include <assert.h>
#include <string.h>

namespace proto{
namespace io{
namespace {
//TODO(kacper): move it out of here
// some explicit instantiation
//#define PROTO_TO_STRING_INSTANTIATE(TYPE)                 \
//    template size_t to_string<TYPE>(char *, size_t, TYPE)

template<typename T>
static auto to_string(char * buffer,
                      size_t max_n, T arg)
    -> typename proto::meta::enable_if<!proto::meta::is_integer<T>::value,
                                       size_t>::type;

template<> [[maybe_unused]]
size_t to_string<const char *>(char * buffer, size_t max_n, const char * arg) {
    size_t arg_len = min(strlen(arg), max_n);
    strncpy(buffer, arg, arg_len);
    return arg_len;
}

template<> [[maybe_unused]]
size_t to_string<const unsigned char *>(char * buffer, size_t max_n,
                                        const unsigned char * arg) {
    return to_string<const char *>(buffer, max_n, (const char *)arg);
}

template<> [[maybe_unused]]
size_t to_string<char *>(char * buffer, size_t max_n, char * arg) {
    return to_string<const char *>(buffer, max_n, (const char *)arg);
}

template<> [[maybe_unused]]
size_t to_string<unsigned char *>(char * buffer, size_t max_n,
                                  unsigned char * arg) {
    return to_string<const char *>(buffer, max_n, (const char *)arg);
}

template<> [[maybe_unused]]
size_t to_string<StringView>(char * buffer, size_t max_n,
                             StringView arg) {
    size_t arg_len = min(arg.length(), max_n);
    strncpy(buffer, arg._str, arg_len);
    return arg_len;
}

// add radix argument
template<typename I> [[maybe_unused]]
auto to_string(char * buffer, size_t max_n, I arg)
    -> typename proto::meta::enable_if<proto::meta::is_integer<I>::value,
                                       size_t>::type
{
    //TODO(kacper): implement log10?
    bool negative = false;
    // if arg is not unsigned
    if(arg < 0) {
        negative = true;
        arg = -arg;
    }

    size_t buflen = arg == 0
        ? 2
        : ceil(log10(arg) + 4);

    char tmpbuf[buflen];

    size_t index = buflen - 1;
    do{
        int decimal_digit = arg % 10;
        arg /= 10;
        tmpbuf[index--] = (char)(decimal_digit + 48);
    } while(arg != 0);

    if(negative)
        tmpbuf[index--] = '-';

    size_t len = buflen - index - 1;

    if( (len + 1) > max_n ) return 0;

    strncpy(buffer, (tmpbuf + index + 1), len); 
    buffer[len] = '\0';

    return len;
}

template<> [[maybe_unused]]
size_t to_string<char>(char * buffer, size_t max_n, char arg) {
    size_t arg_len = max(max_n, 1);
    *buffer = arg;
    return arg_len;
}

template<> [[maybe_unused]]
size_t to_string<bool>(char * buffer, size_t max_n, bool arg) {
    size_t arg_len = (arg) ? 4 : 5;
    assert(arg_len <= max_n);
    strncpy(buffer,
            (arg ? "true" : "false"),
            arg_len);
    return arg_len;
}


//PROTO_TO_STRING_INSTANTIATE(int);

template<typename T>
size_t sprint(char * buffer, size_t max_n, T arg) {
    size_t len = to_string(buffer, max_n, arg);
    buffer += len;
    return len;
}

template<typename T, typename ...Ts>
size_t sprint(char * buffer, size_t max_n, T arg, Ts... args) {
    size_t len = sprint(buffer, max_n, arg);
    return len + sprint(buffer + len, max_n - len, args...);
}

template<typename ...Ts>
void print(Ts... args) {
    //TODO(kacper):
    // make this buffer static assuming this function
    // is not going to be called from different threads
    static char buffer[16384];
    size_t len = sprint(buffer, 2023, args...);
    buffer[len] = '\0';
    printf("%s", buffer);
}

template<typename ...Ts>
void println(Ts... args) {
    print(args..., '\n');
}
 


void flush() {
    fflush(stdout);
}

} // namespace 
} // namespace io
} // namespace proto
