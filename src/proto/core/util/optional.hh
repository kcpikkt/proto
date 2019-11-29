#pragma once
#include <assert.h>

namespace proto
{
    template<typename T>
    struct optional{
        T value;
        bool has_value = false;
    };
}

