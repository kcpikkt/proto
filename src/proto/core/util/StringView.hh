#pragma once
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


        u64 _size;
        const char * _str = nullptr;

        inline Iterator begin() {
            return Iterator(*this, 0);
        }

        inline Iterator end() {
            return Iterator(*this, _size);
        }

        const char * str() {
            return _str;
        }

        u64 size() {
            return _size;
        }

        u64 length() {
            return _size;
        }

        operator bool() {
            return (bool)_str;
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
        StringView(const unsigned char * str)
            : StringView((const char*)str)
        {}



        void remove_suffix(u64 n) {
            assert(n <= _size);
            _size -= n;
        }

    };
} // namespace proto
