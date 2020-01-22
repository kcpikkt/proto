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

            Iterator& operator++() {
                _index++; return *this;
            }

            Iterator operator++(int) {
                Iterator ret = *this;
                _index++; return ret;
            }

            char operator*(){
                return _view[_index];
            }

            bool operator!=(Iterator& other) {
                return 
                    other._view._str != _view._str || other._index != _index;
            }
        };


        const char * _str = nullptr;
        u64 _size;

        inline Iterator begin() {
            return Iterator(*this, 0);
        }

        inline Iterator end() {
            return Iterator(*this, _size);
        }

        constexpr const char * str() {
            return _str;
        }

        constexpr u64 size() {
            return _size;
        }

        constexpr u64 length() {
            return _size;
        }

        operator bool() {
            return (bool)_str;
        }
        bool is_cstring() {
            return (strlen(_str) == _size);
        }

        char operator[](u64 index) {
            assert(_str);
            proto_assert(index < _size);
            return _str[index];
        }
        // NOTE(kacper): I am not sure I want it to be implicit
        // FIXME(kacper): no, it should not, remove it
        operator const char *() {
            return _str;
        }

        constexpr StringView() : _str(nullptr), _size(0) {}

        constexpr StringView(const char * str, u64 size) : _str(str), _size(size) {
            assert(str);
        }

        StringView(const char * str) : StringView(str, strlen(str)) {}

        StringView(const unsigned char * str) : StringView((const char*)str) {}

        inline StringView trim_prefix(u64 n) {
            assert(n <= _size);
            _size -= n; _str += n;
            return StringView(_str - n, n);
        }

        inline StringView trim_suffix(u64 n) {
            assert(n <= _size);
            _size -= n;
            return StringView(_str + _size, n);
        }

        inline StringView prefix(u64 n) {
            assert(n <= _size);
            return StringView(_str, n);
        }

        inline StringView suffix(u64 n) {
            assert(n <= _size);
            return StringView(_str + _size - n, n);
        }

    };
} // namespace proto
