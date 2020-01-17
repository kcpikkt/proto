#pragma once
#include <assert.h>

namespace proto
{
    template<typename T>
    struct Optional{
        T value;
        bool has_value = false;

        operator bool() {
            return has_value;
        }

        Optional() : has_value(false) {}
        Optional(const T& val) : value(val), has_value(true) {}

        Optional<T>& operator=(const T& val) {
            value = val; has_value = true; return *this;
        }

        //operator T&() {
        //    assert(has_value);
        //    return value;
        //}
    };
}

