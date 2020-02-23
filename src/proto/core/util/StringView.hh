#pragma once
#include <string.h>
#include "proto/core/common.hh"
#include "proto/core/debug/assert.hh"

namespace proto {
    
    struct StringView {
        // to allow for range-for
        struct Iterator {
            u64 _index;
            StringView& _view;

            Iterator(StringView& view, u64 index)
                :  _index(index), _view(view) {}

            Iterator& operator++();

            Iterator operator++(int);

            char operator*();

            bool operator!=(Iterator& other);
        };


        const char * str = nullptr;
        u64 length;

        Iterator begin();

        Iterator end();

        operator bool() {
            return (bool)str;
        }

        bool is_cstring();

        char operator[](u64 index);
        // NOTE(kacper): I am not sure I want it to be implicit
        // FIXME(kacper): no, it should not, remove it
        operator const char *() {
            return str;
        }

        constexpr StringView() : str(nullptr), length(0) {}

        constexpr StringView(const char * str, u64 length) : str(str), length(length) {
            assert(str);
        }

        StringView(const char * str) : StringView(str, strlen(str)) {}

        StringView(const unsigned char * str) : StringView((const char*)str) {}

        StringView trim_prefix(u64 n);

        StringView trim_suffix(u64 n);

        StringView prefix(u64 n);

        StringView suffix(u64 n);
    };

    bool operator==(StringView a, StringView b);
} // namespace proto
