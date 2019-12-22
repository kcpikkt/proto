#pragma once

namespace proto {

template<typename T>
struct ModCounter {
    T _mod_value = 0;
    T _value = 0;

    void init(T mod_value) { _mod_value = mod_value; }

    operator T(){ return _value; }

    const T& operator++() {
        _value = (_value + 1) % _mod_value;
        return _value;
    }

    T operator++(int) {
        T prev_value = _value;
        _value = (_value + 1) % _mod_value;
        return prev_value;
    }
};

} // namespace proto
