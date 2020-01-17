#pragma once
#include "proto/core/util/Bitfield.hh"
#include "proto/core/common/types.hh"
#include "proto/core/debug/markers.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/debug/stacktrace.hh"
#include "proto/core/common.hh"
#include "proto/core/error-handling.hh"
//NOTE(kacper): State is a class that have potential to hold
//              external resources - that have pointers to heap.
//              Should not allow for implicit compies, only moves
//              if state was moved from only shallow destructor is called.

namespace proto {

namespace meta {
    template<typename T, typename = void> struct has_state : false_t {};
    template<typename T> struct has_state<T, void_t<typename T::_StateTestingProbe>> : true_t {};

    template<typename T>
    constexpr static auto has_state_v = has_state<T>::value;
}

namespace constant {
} // namespace constant

struct DefaultStateErrCategoryExt : ErrCategoryCRTP<DefaultStateErrCategoryExt> {
    constexpr static ErrMessage message(ErrCode) { return "Unknown error"; }
};

template<typename T, typename ErrCategoryExt = DefaultStateErrCategoryExt>
struct StateCRTP  {

    // this err category is a member cause user can extend this by ErrCategoryExt
    // Not sure how good/bad of an idea this is
    struct ErrCategory : ErrCategoryCRTP<ErrCategory>, ErrCategoryExt {
        enum : ErrCode {
            success = 0,
            double_deep_destroy,
            double_shallow_destroy,
            out_of_scope_leak,
            destroy_uninitialized,
            destroy_deep_unimplemented,
            destroy_fail,
            // not actually used by base StateCRTP but useful so there is no need for extensions
            free_fail,
            file_close_fail,
        };

        constexpr static ErrMessage message(ErrCode code) {
            switch(code) {
            case double_deep_destroy:
                return "Double destroying stateful object (after it was deep-destroyed), no destruction performed.";
            case double_shallow_destroy:
                return "Double destroying stateful object (after it was shallow-destroyed), no destruction performed.";
            case out_of_scope_leak:
                return "No destruction performed on initialized object going out of scope, possible leak.";
            case destroy_uninitialized:
                return "Destructor method called on unitialized state object, no destruction performed.";
            case destroy_deep_unimplemented:
                return "Type inheriting StateCRTP does not implement destroy_deep() method.";
            case destroy_fail:
                return "Destructon failed due to... some reasons? catchall error, sry for being lazy, fixme";
            case free_fail:
                return "Failed to free memory.";
            case file_close_fail:
                return "Failed to close file.";
            }
            return ErrCategoryExt::message(code); // perhaps it is user side error, from ErrCategoryExt(ension) but idk
        }
    };

    enum : u8 {
        _initialized_bit       = BIT(0),
        _moved_bit             = BIT(1),
        _shallow_destroyed_bit = BIT(2), 
        _deep_destroyed_bit    = BIT(3),
        _autodestruct_bit      = BIT(4),
    };

    Bitfield<u8> state_flags = 0;

    using State = StateCRTP<T>;
    using _StateTestingProbe = void;

    inline bool is_moved() {
        return state_flags.check(_moved_bit); }

    inline bool is_initialized() {
        return state_flags.check(_initialized_bit); }

    inline void set_autodestruct() {
        state_flags.set(_autodestruct_bit);}

    //NOTE(kacper): remember to forward;
    void state_move(T&& other);

    void state_copy(const T& other);

    // call that in every init/constructor/assignment
    void state_init([[maybe_unused]] bool assignment = false);

    Err<ErrCategory> _destroy_shallow();
    Err<ErrCategory> _destroy_deep();

    //NOTE(kacper): Dummies, idk if I want them or force implementation
    //NOTE(kacper): Perhaps add warning on unimplemented deep dtor
    Err<ErrCategory> destroy_shallow() { return StateCRTP<T>::ErrCategory::success; }
    Err<ErrCategory> destroy_deep() {
        Err<ErrCategory> err = ErrCategory::destroy_deep_unimplemented;
        debug_error(debug::category::data, err.message());
        return err;
    }

    Err<ErrCategory> destroy();

    ~StateCRTP();
};

template<typename T, typename Ext = DefaultStateErrCategoryExt>
using StateErrCategory = typename StateCRTP<T, Ext>::ErrCategory;

template<typename T, typename Ext = DefaultStateErrCategoryExt>
using StateErr = Err<StateErrCategory<T, Ext>>;

} // namespace proto

//IMPL

namespace proto {

template<typename T, typename Ext>
void StateCRTP<T, Ext>::state_move(T&& other) {
    state_init(true);
    state_flags = other.state_flags;
    other.state_flags.set(_moved_bit);
}

template<typename T, typename Ext>
void StateCRTP<T, Ext>::state_copy(const T& other) {
    state_init(true);
    state_flags = other.state_flags;
}

// call that in every init/constructor
template<typename T, typename Ext>
void StateCRTP<T, Ext>::state_init([[maybe_unused]] bool assignment) {
    assert(!state_flags.check(_initialized_bit) || assignment);
    state_flags.set(_initialized_bit);
}

template<typename T, typename Ext>
StateErr<T, Ext> StateCRTP<T, Ext>::_destroy_shallow() {
    assert(!state_flags.check(_shallow_destroyed_bit));

    auto err = static_cast<T*>(this)->destroy_shallow();

    if(err == ErrCategory::success)
        state_flags.set(_shallow_destroyed_bit);

    return err;
}

template<typename T, typename Ext>
StateErr<T, Ext> StateCRTP<T, Ext>::_destroy_deep() {
    assert(!state_flags.check(_deep_destroyed_bit));

    static_assert(meta::is_same_v<decltype(meta::declval<T>().destroy_deep()), StateErr<T, Ext> >,
                  "destory_deep() method has wrong return type.");
    auto err = static_cast<T*>(this)->destroy_deep();

    if(err == ErrCategory::success)
        state_flags.set(_deep_destroyed_bit);

    return err;
}

template<typename T, typename Ext>
StateErr<T, Ext> StateCRTP<T, Ext>::destroy() {
    using ErrCategory = typename StateCRTP<T, Ext>::ErrCategory;

    StateErr<T, Ext> err = ErrCategory::success;

#if defined(PROTO_DEBUG)
    if(state_flags.check(_deep_destroyed_bit)) {
        err = ErrCategory::double_deep_destroy;
        debug_error(debug::category::data, err.message());
        return err; 
    }

    if(state_flags.check(_shallow_destroyed_bit)) {
        err = ErrCategory::double_shallow_destroy;
        debug_error(debug::category::data, err.message());
        return err;
    }
#endif

    if(state_flags.check(_initialized_bit)) {
        err = _destroy_shallow();
        if(!is_moved())
            err = _destroy_deep();
    } else {
        #if 1
        if constexpr(meta::is_base_of_v<debug::Marker, T>) {
            log_debug_marker(debug::category::main,
                             (*static_cast<T*>(this)));
        }
        #endif
        debug_warn(debug::category::main, ErrCategory::message(ErrCategory::destroy_uninitialized));
        debug::stacktrace();
    }

    if(!err) state_flags.unset(_initialized_bit);
    return err;
}

template<typename T, typename Ext>
StateCRTP<T, Ext>::~StateCRTP() {
    using ErrCategory = typename StateCRTP<T, Ext>::ErrCategory;

    if(state_flags.check(_autodestruct_bit)) destroy();

#if defined(PROTO_DEBUG)
    if(state_flags.check(_initialized_bit) && !state_flags.check(_moved_bit)) {
        if(!state_flags.check(_deep_destroyed_bit)) {
            debug_warn(debug::category::data, ErrCategory::message(ErrCategory::out_of_scope_leak));
            return;
        }
    }
#endif
}

} // namespace proto

