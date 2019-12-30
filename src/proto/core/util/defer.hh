#pragma once
#include "proto/core/preprocessor.hh"

// stolen from https://www.gingerbill.org/article/2015/08/19/defer-in-cpp/

template <typename F>
struct _Deferred {
    F f;
    _Deferred(F f) : f(f) {}
    ~_Deferred() { f(); }
};

#define defer _Deferred CONCAT(_deferred_call_, __COUNTER__) = [&]()
