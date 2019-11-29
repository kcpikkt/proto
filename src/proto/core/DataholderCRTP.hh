#pragma once
#include "proto/core/util/Bitfield.hh"
#include "proto/core/common/types.hh"

namespace proto {

template<typename T>
struct DataholderCRTP {
    Bitfield<u8> flags;
    constexpr static u8 _initialized_bit       = 1;
    constexpr static u8 _moved_bit             = 2;
    constexpr static u8 _shallow_destroyed_bit = 4;
    constexpr static u8 _deep_destroyed_bit    = 8;

    inline bool is_moved() { return flags.check(_moved_bit); }
    inline bool is_initialized() { return flags.check(_initialized_bit); }

    //NOTE(kacper): remember to forward;
    void move(T&& other) {
        init_base(true);
        flags = other.flags;
        other.flags.set(_moved_bit);
    }

    void copy(const T& other) {
        init_base(true);
        flags = other.flags;
    }

    // call that in every init/constructor/assignment
    void init_base([[maybe_unused]] bool assignment = false) {
        assert(!flags.check(_initialized_bit) || assignment);
        flags.set(_initialized_bit);
    }

    void _destroy_shallow() {
        assert(!flags.check(_shallow_destroyed_bit));
        static_cast<T*>(this)->destroy_shallow();
        flags.set(_shallow_destroyed_bit);
    }

    void _destroy_deep() {
        assert(!flags.check(_deep_destroyed_bit));
        static_cast<T*>(this)->destroy_deep();
        flags.set(_deep_destroyed_bit);
    }

    //NOTE(kacper): Dummies, idk if I want them or force implementation
    //NOTE(kacper): Perhaps add warning on unimplemented deep dtor
    void destroy_shallow() {}
    void destroy_deep() {}

    // stick it in the dtor
    void destroy() {
        assert(flags.check(_initialized_bit));
        _destroy_shallow();
        if(!is_moved())
            _destroy_deep();
    }

    ~DataholderCRTP() {
        destroy();
    }
};

} // namespace proto
