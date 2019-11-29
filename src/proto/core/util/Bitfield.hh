#pragma once
namespace proto {
    template<typename T>
    struct Bitfield {
        T _field = 0;
        void set(T option) { _field |= option; }
        void unset(T option) { _field &= ~option; }
        bool check(T option) { return (_field & option); }
    };
} // namespace proto

