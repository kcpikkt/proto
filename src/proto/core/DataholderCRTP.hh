#pragma once
#include "proto/core/util/Bitfield.hh"
#include "proto/core/common/types.hh"
#include "proto/core/debug/markers.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/common.hh"
//NOTE(kacper): Dataholder is a class that have potential to hold
//              external resources - that have pointers to heap.
//              Should rather not be copied only moved,
//              if dataholder was moved only shallow destructor is called.

namespace proto {

template<typename T>
struct DataholderCRTP  {
    Bitfield<u8> dataholder_flags;
    constexpr static u8 _initialized_bit       = BIT(1);
    constexpr static u8 _moved_bit             = BIT(2);
    constexpr static u8 _shallow_destroyed_bit = BIT(3);
    constexpr static u8 _deep_destroyed_bit    = BIT(4);

    inline bool is_moved() {
        return dataholder_flags.check(_moved_bit); }

    inline bool is_initialized() {
        return dataholder_flags.check(_initialized_bit); }

    //NOTE(kacper): remember to forward;
    void dataholder_move(T&& other);

    void dataholder_copy(const T& other);

    // call that in every init/constructor/assignment
    void dataholder_init([[maybe_unused]] bool assignment = false);

    void _destroy_shallow();

    void _destroy_deep();

    //NOTE(kacper): Dummies, idk if I want them or force implementation
    //NOTE(kacper): Perhaps add warning on unimplemented deep dtor
    void destroy_shallow() {}
    void destroy_deep() {}

    // stick it in the dtor
    void destroy();

    ~DataholderCRTP();
};

} // namespace proto

//IMPL

namespace proto {
template<typename T>
void DataholderCRTP<T>::dataholder_move(T&& other) {
    dataholder_init(true);
    dataholder_flags = other.dataholder_flags;
    other.dataholder_flags.set(_moved_bit);
}

template<typename T>
void DataholderCRTP<T>::dataholder_copy(const T& other) {
    dataholder_init(true);
    dataholder_flags = other.dataholder_flags;
}

// call that in every init/constructor/assignment
template<typename T>
void DataholderCRTP<T>::dataholder_init([[maybe_unused]] bool assignment) {
    assert(!dataholder_flags.check(_initialized_bit) || assignment);
    dataholder_flags.set(_initialized_bit);
}

template<typename T>
void DataholderCRTP<T>::_destroy_shallow() {
    assert(!dataholder_flags.check(_shallow_destroyed_bit));
    static_cast<T*>(this)->destroy_shallow();
    dataholder_flags.set(_shallow_destroyed_bit);
}

template<typename T>
void DataholderCRTP<T>::_destroy_deep() {
    assert(!dataholder_flags.check(_deep_destroyed_bit));
    static_cast<T*>(this)->destroy_deep();
    dataholder_flags.set(_deep_destroyed_bit);
}

// stick it in the dtor
template<typename T>
void DataholderCRTP<T>::destroy() {
    if(dataholder_flags.check(_initialized_bit)) {
        _destroy_shallow();
        if(!is_moved())
            _destroy_deep();
    } else {
        // NOTE(kacper): Let me disable this warning until I fix
        //               all unitialized dataholders.
        // TOOD(kacper): switches to disable certain types of messages
        #if 0
        if constexpr(meta::is_base_of_v<debug::Marker, T>) {
            log_debug_marker(debug::category::main,
                             (*static_cast<T*>(this)));
        }
        debug_warn(debug::category::main,
                   "destructor called on unitialized dataholder object, ",
                   "no destruction performed.");
        #endif
    }
}

template<typename T>
DataholderCRTP<T>::~DataholderCRTP() {
    destroy();
}

} // namespace proto

