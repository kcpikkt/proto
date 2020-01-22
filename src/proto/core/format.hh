#pragma once
#include "proto/core/util/StringView.hh"
#include "proto/core/util/StringView.hh"

namespace proto {
// this function returns pointer to internal buffer that is rewritten
// and may in future be even reallocated so DONT KEEP ITS RETURN VAL, PRINT IT OR COPY IMMEDIATELY AFTER USE
// or you will get wacky, stupid, confusing bugs, thanks

//template<typename T> 
//StringView _format(char * buffer, u64 max_len, StringView fmt, T arg) {
//}
template<typename T> 
u32 _format(MemBuffer buf, StringView& fmt, T arg) {
    if(buf.size == 0) return 0;

    for(u32 i=0; i < fmt.length(); ++i) {
        char c = fmt[i];
        //TODO(kacper): escaped %%%
        if(c == '%') {
            u32 len, acc;

            len = acc = min(buf.size, i); // buffer.size - 1 to accomodate for \0, buffer.size is never 0 here
            strview_copy((char*)buf.data8, fmt.trim_prefix(len));
            (void)fmt.trim_prefix(1); // percent sign

            len += to_string((char*)(buf.data8 + len), (buf.size - len), arg);
            return len;
        }
    }

    //debug_error(debug::category::main, "too few arguments for the formatting function");
    return 0;
}


template<typename T, typename ...Ts>
u32 _format(MemBuffer buf, StringView& fmt, T arg, Ts... args) {
    if(buf.size == 0) return 0;

    u32 len = 0;
    len += _format(buf, /*ref*/ fmt, arg);
    buf.data8 += len; buf.size -= len;

    len += _format(buf, /*ref*/ fmt, args...);
    return len;
}

template<typename ...Ts>
StringView format(StringView fmt, Ts... args) {
    static constexpr u64 bufsz = 1024 * 64;
    static char buffer[bufsz]; //temp

    u32 written = _format( {{buffer}, bufsz}, /*ref*/ fmt, args... );
    u32 len = min(bufsz - written, fmt.length());
    strview_copy(buffer + written, fmt.prefix(len));
    written += len;

    return StringView(buffer, written);
}

} // namespace proto

