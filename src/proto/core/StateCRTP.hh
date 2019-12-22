#pragma once
#include "proto/core/util/Bitfield.hh"
#include "proto/core/common/types.hh"
#include "proto/core/debug/markers.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/common.hh"
//NOTE(kacper): State is a class that have potential to hold
//              external resources - that have pointers to heap.
//              Should rather not be copied only moved,
//              if state was moved from only shallow destructor is called.

namespace proto {

template<typename T>
struct StateCRTP  {
    Bitfield<u8> state_flags;
    constexpr static u8 _initialized_bit       = BIT(1);
    constexpr static u8 _moved_bit             = BIT(2);
    constexpr static u8 _shallow_destroyed_bit = BIT(3);
    constexpr static u8 _deep_destroyed_bit    = BIT(4);

    using State = StateCRTP<T>;

    inline bool is_moved() {
        return state_flags.check(_moved_bit); }

    inline bool is_initialized() {
        return state_flags.check(_initialized_bit); }

    //NOTE(kacper): remember to forward;
    void state_move(T&& other);

    void state_copy(const T& other);

    // call that in every init/constructor/assignment
    void state_init([[maybe_unused]] bool assignment = false);

    void _destroy_shallow();

    void _destroy_deep();

    //NOTE(kacper): Dummies, idk if I want them or force implementation
    //NOTE(kacper): Perhaps add warning on unimplemented deep dtor
    void destroy_shallow() {}
    void destroy_deep() {}

    // stick it in the dtor
    void destroy();

    ~StateCRTP();
};

} // namespace proto

//IMPL

namespace proto {
template<typename T>
void StateCRTP<T>::state_move(T&& other) {
    state_init(true);
    state_flags = other.state_flags;
    other.state_flags.set(_moved_bit);
}

template<typename T>
void StateCRTP<T>::state_copy(const T& other) {
    state_init(true);
    state_flags = other.state_flags;
}

// call that in every init/constructor/assignment
template<typename T>
void StateCRTP<T>::state_init([[maybe_unused]] bool assignment) {
    assert(!state_flags.check(_initialized_bit) || assignment);
    state_flags.set(_initialized_bit);
}

template<typename T>
void StateCRTP<T>::_destroy_shallow() {
    assert(!state_flags.check(_shallow_destroyed_bit));
    static_cast<T*>(this)->destroy_shallow();
    state_flags.set(_shallow_destroyed_bit);
}

template<typename T>
void StateCRTP<T>::_destroy_deep() {
    assert(!state_flags.check(_deep_destroyed_bit));
    static_cast<T*>(this)->destroy_deep();
    state_flags.set(_deep_destroyed_bit);
}

// stick it in the dtor
template<typename T>
void StateCRTP<T>::destroy() {
    if(state_flags.check(_initialized_bit)) {
        _destroy_shallow();
        if(!is_moved())
            _destroy_deep();
    } else {
        // NOTE(kacper): Let me disable this warning until I fix
        //               all unitialized states.
        // TOOD(kacper): switches to disable certain types of messages
        #if 0
        if constexpr(meta::is_base_of_v<debug::Marker, T>) {
            log_debug_marker(debug::category::main,
                             (*static_cast<T*>(this)));
        }
        debug_warn(debug::category::main,
                   "destructor called on unitialized state object, ",
                   "no destruction performed.");
        #endif
    }
}

template<typename T>
StateCRTP<T>::~StateCRTP() {
    destroy();
}

} // namespace proto

