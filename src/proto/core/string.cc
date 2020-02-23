#include "proto/core/string.hh"
#include "proto/core/util/algo.hh"
#include "proto/core/util/Bitset.hh"
#include "proto/core/io.hh"

namespace proto {

template<typename I> // for all strings
auto to_string_integer(char * buffer, u64 max_len, I arg) ->
    meta::enable_if_t<meta::is_integer_v<I> && (sizeof(I) > 1), u64>
{
    //TODO(kacper): implement log10?
    bool negative = false;
    // if arg is not unsigned
    if(arg < 0) {
        negative = true;
        arg = -arg;
    }

    size_t buflen = (arg == 0) ? 2 : ceil(log10(arg) + 4);

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

    if( (len + 1) > max_len ) return 0;

    strncpy(buffer, (tmpbuf + index + 1), len); 
    buffer[len] = '\0';

    return len;
}

// for all strings
template<>
u64 to_string_specific<StringView>(char * buffer, u64 max_len, StringView arg)
{
    u64 len = min(arg.length, max_len - 1);
    memcpy(buffer, arg, len);
    buffer[len] = '\0';
    return len;
}


template<> 
u64 to_string_specific<u8>(char * buffer, u64 max_len, u8 arg){
    u64 len = min(1, max_len - 1);
    (*buffer) = arg;
    buffer[len] = '\0';
    return len;
}

template<>
u64 to_string_specific<ivec2>(char * buffer, u64 max_len, ivec2 arg) {
    return sprint(buffer, max_len,
                  '(', arg.x, ',', arg.y, ')');
}

template<>
u64 to_string_specific<ivec3>(char * buffer, u64 max_len, ivec3 arg) {
    return sprint(buffer, max_len,
                  '(', arg.x, ',', arg.y, ',', arg.z, ')');
}

template<>
u64 to_string_specific<ivec4>(char * buffer, u64 max_len, ivec4 arg) {
    return sprint(buffer, max_len,
                  '(', arg.x, ',', arg.y, ',', arg.z, ',', arg.w, ')');
}


template<typename T>
u64 to_string(char * buffer, u64 max_len, T arg)
{
    if(max_len == 0) return 0;
    /*  */ if constexpr(meta::is_integer_v<T> && (sizeof(T) > 1)) {
        return to_string_integer(buffer, max_len, arg);
    } else if constexpr(meta::is_convertible_v<T, StringView>) {
        return to_string_specific(buffer, max_len, StringView(arg));
    } else if constexpr(meta::is_convertible_v<T, u8>){
        return to_string_specific(buffer, max_len, (u8)arg);
    } else {
        return to_string_specific(buffer, max_len, arg);
    }
}

// TODO(kacper): incorporate it into to_string
template<typename T>
u64 sprint(char * buffer, u64 max_len, T arg) {
    u64 len = to_string(buffer, max_len, arg);
    buffer += len;
    return len;
}


// NOTE(kacper): can you make meta::typelist of types and explicity instantiate based on it??
template u64 sprint<char> (char*,u64,char );

template u64 sprint<u8> (char*, u64, u8 );
template u64 sprint<u16>(char*, u64, u16);
template u64 sprint<u32>(char*, u64, u32);
template u64 sprint<u64>(char*, u64, u64);

template u64 sprint<s8> (char*, u64, s8 );
template u64 sprint<s16>(char*, u64, s16);
template u64 sprint<s32>(char*, u64, s32);
template u64 sprint<s64>(char*, u64, s64);

using uchar = unsigned char;

template u64 sprint< char*>       (char*, u64, char*);
template u64 sprint<uchar*>       (char*, u64, uchar*);
template u64 sprint<const  char*> (char*, u64, const char*);
template u64 sprint<const uchar*> (char*, u64, const uchar*);
template u64 sprint<StringView>   (char*, u64, StringView);

template u64 sprint<ivec2> (char*, u64, ivec2);
template u64 sprint<ivec3> (char*, u64, ivec3);
template u64 sprint<ivec4> (char*, u64, ivec4);

char * strview_copy(char * dest, StringView src) {
    memcpy(dest, src, src.length);
    dest[src.length] = '\0';
    return dest;
}

char * strview_cat(char * dest, StringView src) {
    u64 destlen = strlen(dest);
    return strview_copy(dest + destlen, src);
}

//NOTE(kacper): btw non capturing lambdas can be casted to function pointers
// this function returns number of elements such that ch[i] != op(ch[i])
//NOTE(kacper): just add caputring lambdas, they are super useful
//TODO(kacper): implement std::function
//TODO(kacper): you have FunctionView now, don't you?
int str_transform(char * str, char(*op)(char)) {
    int count = 0; char prev;

    for(; (prev = *str) != '\0'; str++)
        count += (prev != (*str = op(*str)) );

    return count;
}

int str_swap(char * str, char from, char to) {
    int count = 0; char prev;

    auto op = [&](char c) { return (c == from ? to : c); };

    for(; (prev = *str) != '\0'; str++)
        count += (prev != (*str = op(*str)) );

    return count;
}

u32 strview_count(StringView str, char c) {
    u32 ret_count = 0;
    for(auto view_c : str)
        if(view_c == c) ret_count++;
    return ret_count;
}

bool strview_cmp(StringView fst, StringView snd) {
    if(fst.length != snd.length) return false;
    u32 index = 0, len = fst.length;
    while( *(fst.str + index) == *(snd.str + index) && index < len) index++;
    return (index == len);
}

bool strview_cmp_i(StringView fst, StringView snd) {
    if(fst.length != snd.length) return false;
    char ch1, ch2;
    u32 index = 0, len = fst.length;;
    while(index < len) {
        ch1 = *(fst.str + index);
        ch2 = *(snd.str + index);
        int diff = ch1 - ch2;
        if((diff == 0)                               ||
           (ch1 >= 'a' && ch1 <= 'z' && diff ==  32) ||
           (ch1 >= 'A' && ch1 <= 'Z' && diff == -32)) index++;
        else break;
    }
    return (index == len);
}

} // namespace proto




