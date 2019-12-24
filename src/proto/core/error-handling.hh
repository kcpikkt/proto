#pragma once
#include "proto/core/util/StringView.hh"
#include "proto/core/meta.hh"

namespace proto {

using ErrCode = s32;
using ErrMessage = StringView;

template<typename T>
struct ErrCategoryCRTP {
    static ErrMessage message(ErrCode code) {
        return T::message(code);
    }
};

template<typename Category>
struct Err {
    static_assert(meta::is_base_of_v<ErrCategoryCRTP<Category>, Category>);

    s32 code;
    operator bool() { return (bool)code; }

    Err(ErrCode code) : code(code) {}

    ErrMessage message() {
        return Category::message(code);
    }
};

} // namespace proto