#pragma once
#include "proto/core/util/StringView.hh"

namespace proto {
static char * strview_copy(char * dest, StringView src) {
    memcpy(dest, src, src.length());
    dest[src.length()] = '\0';
    return dest;
}

static char * strview_cat(char * dest, StringView src) {
    u64 destlen = strlen(dest);
    return strview_copy(dest + destlen, src);
}


//NOTE(kacper): btw non capturing lambdas can be casted to function pointers
// this function returns number of elements such that ch[i] != op(ch[i])
//NOTE(kacper): just add caputring lambdas, they are super useful
//TODO(kacper): implement std::function
static int str_trans(char * str, char(*op)(char)) {
    int count = 0; char prev;

    for(; (prev = *str) != '\0'; str++)
        count += (prev != (*str = op(*str)) );

    return count;
}

static int str_swap(char * str, char from, char to) {
    int count = 0; char prev;

    auto op = [&](char c) { return (c == from ? to : c); };

    for(; (prev = *str) != '\0'; str++)
        count += (prev != (*str = op(*str)) );

    return count;
}

static u32 strview_count(StringView str, char c) {
    u32 ret_count = 0;
    for(auto view_c : str)
        if(view_c == c) ret_count++;
    return ret_count;
}
} // namespace proto




