#pragma once
#include "proto/core/meta.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/common/types.hh"

namespace proto {

template<typename I> // for all strings
auto to_string_integer(char * buffer, u64 max_len, I arg) ->
    meta::enable_if_t<meta::is_integer_v<I> && (sizeof(I) > 1), u64>;

template<typename T>
u64 to_string_specific(char * buffer, u64 max_len, T arg);

template<typename T>
u64 to_string(char * buffer, u64 max_len, T arg);

template<typename T>
u64 sprint(char * buffer, u64 max_len, T arg);

template<typename T, typename ...Ts>
static u64 sprint(char * buffer, u64 max_len, T arg, Ts... args) {
    u64 len = sprint(buffer, max_len, arg);
    return len + sprint(buffer + len, max_len - len, args...);
}

char * strview_copy(char * dest, StringView src);
char * strview_cat(char * dest, StringView src);

//NOTE(kacper): btw non capturing lambdas can be casted to function pointers
// this function returns number of elements such that ch[i] != op(ch[i])
//NOTE(kacper): just add caputring lambdas, they are super useful
//TODO(kacper): implement std::function
int str_transform(char * str, char(*op)(char));

int str_swap(char * str, char from, char to);

u32 strview_count(StringView str, char c);

} // namespace proto




