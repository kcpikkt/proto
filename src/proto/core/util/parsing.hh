#pragma once
#include "proto/core/util/StringView.hh"

namespace proto {
namespace {

    bool is_eof(char * c, char * begin, size_t size){
        if(*c == '\0') {
            assert((size_t)(c - begin) == size);
            return true;
        }
        return false;
    };

    bool is_space (char c){
        return (c == ' ' || c == '\t');
    };

    bool is_digit(char c){
        return (48 <= c && c < 58);
    };

    bool is_white(char c){
        return (c == ' '  || c == '\n' ||
                c == '\r' || c == '\t' ||
                c == '\f' || c == '\v');
    };    

    void next_line(char ** _ptr, char * begin, size_t size){
        char * ptr = *_ptr;

        while(ptr < (begin + size)) {
            if(*ptr == '\n') {
                // skip empty lines
                do{ ptr++; } while(*ptr == '\n');
                break;
            }

            if(is_eof(ptr, begin, size)) break;
            ptr++;
        }
        *_ptr = ptr;
    };
    float extract_float(char ** _ptr){
        char * ptr = *_ptr;
        //skip initial spaces
        while(is_white(*ptr)) ptr++;
        char * float_str = ptr;
        while(!is_white(*ptr)) ptr++;
        char prev_char = *ptr;
        *(ptr) = '\0';
        float ret = (float)atof(float_str);
        *(ptr) = prev_char;
        *_ptr = ptr;
        return ret;
    };

    vec3 extract_vec3(char ** _ptr){
        vec3 ret;
        ret.x = extract_float(_ptr);
        ret.y = extract_float(_ptr);
        ret.z = extract_float(_ptr);
        return ret;
    };

    int extract_int(char ** _ptr){
        char * ptr = *_ptr;
        //skip initial spaces
        while(is_white(*ptr)) ptr++;
        char * int_str = ptr;
        while(is_digit(*ptr)) ptr++;
        char prev_char = *ptr;
        *(ptr) = '\0';
        int ret = (int)atoi(int_str);
        assert(ret >= 0);
        *(ptr) = prev_char;
        // skip to next 
        //while(!is_white(*ptr)) ptr++;
        *_ptr = ptr;
        return ret;
    };

    StringView extract_name (char ** _ptr){
        int _len = 0;
        char * ptr = *_ptr;
        while(is_space(*ptr)) ptr++;
        const char * name_str = ptr;
        while(!is_white( *(ptr + (_len)) ))_len++;
        ptr = ptr + _len;

        _ptr = &ptr;

        return StringView(name_str, _len);
    };


} // namespace <anonymous>
} // namespace proto
