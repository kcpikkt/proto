#pragma once
namespace proto {
    template<typename T>
    struct Bitfield {
        T _field = 0;
        void set   (T option) { _field |= option; }
        void unset (T option) { _field &= ~option; }
        void toggle(T option) { _field ^= option; }
        bool check (T option) const { return (_field & option); }

        Bitfield() : _field(0) {}

        ~Bitfield() {}

        Bitfield(T value) {
            _field = value;}

        Bitfield<T>& operator=(T value) {
            _field = value;
            return *this;
        }

        Bitfield(const Bitfield<T>& other) {
            _field = other._field;}

        Bitfield<T> operator=(Bitfield<T>& other) {
            _field = other._field;
            return *this;
        }
    };
} // namespace proto

