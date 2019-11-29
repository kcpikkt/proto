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
static int str_t(char * str, char(*op)(char)) {
    int count = 0; char prev;

    for(; (prev = *str) != '\0'; str++)
        count += (prev != (*str = op(*str)) );

    return count;
}

} // namespace proto




