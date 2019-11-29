#pragma once
#include "proto/core/common.hh"

namespace proto {
    
    struct StringView {
        u64 _size;
        const char * _str = nullptr;

        const char * str() {
            return _str;
        }

        u64 size() {
            return _size;
        }

        u64 length() {
            return _size;
        }

        // NOTE(kacper) I am not sure I want it to be implicit
        operator const char *() {
            return _str;
        }

        StringView() {}

        StringView(const char * str, u64 size) {
            assert(str);
            _str = str;
            _size = size;
        }

        StringView(const char * str) {
            assert(str);
            _str = str;
            _size = strlen(str);
        }

        void remove_suffix(u64 n) {
            assert(n <= _size);
            _size -= n;
        }

    };
} // namespace proto
