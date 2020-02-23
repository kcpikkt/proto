#pragma once
#include "proto/core/meta.hh"
#include "proto/core/util/algo.hh"
#include "proto/core/platform/api.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/string.hh"
#include "proto/core/format.hh"
#include <stdio.h>
#include <tgmath.h>
#include <assert.h>
#include <string.h>

namespace proto{
namespace {
//TODO(kacper): move it out of here
// some explicit instantiation
//#define PROTO_TO_STRING_INSTANTIATE(TYPE)                 \
//    template size_t to_string<TYPE>(char *, size_t, TYPE)

template<typename ...Ts>
void print(Ts... args) {
    //TODO(kacper):
    // make this buffer static assuming this function
    // is not going to be called from different threads
    static char buffer[16384];
    size_t len = sprint(buffer, 16384, args...);
    buffer[len] = '\0';
    printf("%.*s", (int)len, buffer);
}

template<typename ...Ts>
void println(Ts... args) {
    print(args..., '\n');
}

template<typename T>
void printn(T arg, u64 n) {
    // TEMPORARY LAZY IMPl
    for(u64 i=0; i<n; ++i) print(arg);
}

template<typename ...Ts>
inline void println_fmt(StringView fmt, Ts... ts) {
    println(format(fmt, ts...));
}

template<typename ...Ts>
inline void print_fmt(StringView fmt, Ts... ts) {
    print(format(fmt, ts...));
}
 
void flush() {
    fflush(stdout);
}

} // namespace 
} // namespace proto
