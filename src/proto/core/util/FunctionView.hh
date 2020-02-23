#pragma once
#include "proto/core/meta.hh"
// based on https://vittorioromeo.info/index/blog/passing_functions_to_functions.html

namespace proto {

template <typename Signature>
struct FunctionView;

template <typename Ret, typename... Args>
struct FunctionView<Ret(Args...)> final {

    using Signature = Ret(void*, Args...);

    void* _ptr;
    Ret (*_erased_function)(void*, Args...);

    //meta::is_callable<T&(TArgs...)>{} && add maybe
    // NO SFINAE JUST Give me callable for gods sake!
    template <typename T,
              // some safety measures, not completely sure...
              typename = meta::enable_if_t<!meta::is_same_v<meta::decay_t<T>, FunctionView>>>
    FunctionView(T&& f) : _ptr{ (void*)&f }
    {
        _erased_function = [](void* ptr, Args... args) -> Ret {
            return (*reinterpret_cast<meta::add_pointer_t<T>>(ptr))(meta::forward<Args>(args)...);
        };
    }

    decltype(auto) operator()(Args... args) const 
    // all my code is noexcept so no need for that
    // noexcept(noexcept(_erased_fn(_ptr, meta::forward<Args>(args)...)))
    {
        return _erased_function(_ptr, meta::forward<Args>(args)...);
    }
};

} // namespace proto
