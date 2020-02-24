#pragma once
#include "proto/core/util/Bitset.hh"
#include "proto/core/common/types.hh"
#include "proto/core/debug/markers.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/debug/stacktrace.hh"
#include "proto/core/common.hh"
#include "proto/core/debug/common.hh"
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

    //struct DefaultStateErrCategoryExt : ErrCategoryCRTP<DefaultStateErrCategoryExt> {
    //    constexpr static ErrMessage message(ErrCode) { return "Unknown error"; }
    //};

template<typename T>
struct StateCRTP  {

    //    // this err category is a member cause user can extend this by ErrCategoryExt
    //    // Not sure how good/bad of an idea this is
    //    struct ErrCategory : ErrCategoryCRTP<ErrCategory>, ErrCategoryExt {
    //        enum : ErrCode {
    //            success = 0,
    //            double_deep_dtor,
    //            double_shallow_dtor,
    //            out_of_scope_leak,
    //            dtor_uninitialized,
    //            dtor_deep_unimplemented,
    //            dtor_fail,
    //            // not actually used by base StateCRTP but useful so there is no need for extensions
    //            free_fail,
    //            file_close_fail,
    //        };
    //
    //        constexpr static ErrMessage message(ErrCode code) {
    //            switch(code) {
    //            case double_deep_dtor:
    //                return "Double dtoring stateful object (after it was deep-dtor), no destruction performed.";
    //            case double_shallow_dtor:
    //                return "Double dtoring stateful object (after it was shallow-dtor), no destruction performed.";
    //            case out_of_scope_leak:
    //                return "No destruction performed on initialized object going out of scope, possible leak.";
    //            case dtor_uninitialized:
    //                return "Destructor method called on unitialized state object, no destruction performed.";
    //            case dtor_deep_unimplemented:
    //                return "Type inheriting StateCRTP does not implement dtor_deep() method.";
    //            case dtor_fail:
    //                return "Destructon failed due to... some reasons? catchall error, sry for being lazy, fixme";
    //            case free_fail:
    //                return "Failed to free memory.";
    //            case file_close_fail:
    //                return "Failed to close file.";
    //            }
    //            return ErrCategoryExt::message(code); // perhaps it is user side error, from ErrCategoryExt(ension) but idk
    //        }
    //    };

    enum : u8 {
        _initialized_bit = 0,
        _moved_bit,
        _shallow_dtor_bit,
        _deep_dtor_bit,
        _defer_dtor_bit,
        _no_dtor_bit,

        ////////////////////
        _bitset_size
    };

    Bitset<_bitset_size> state_flags;

    using State = StateCRTP<T>;
    using _StateTestingProbe = void;

    inline bool is_moved() {
        return state_flags.at(_moved_bit); }

    inline bool is_initialized() {
        return state_flags.at(_initialized_bit); }

    inline void no_dtor() {
        state_flags.set(_no_dtor_bit); }

    inline void defer_dtor() {
        PROTO_DEPRECATED;
        state_flags.set(_defer_dtor_bit);
    }

    //NOTE(kacper): remember to forward;
    void state_move(T&& other);

    void state_copy(const T& other);

    // call that in every init/constructor/assignment
    void state_init([[maybe_unused]] bool assignment = false);

    Err _dtor_shallow();
    Err _dtor_deep();

    //NOTE(kacper): Dummies, idk if I want them or force implementation
    //NOTE(kacper): Perhaps add warning on unimplemented deep dtor
    Err dtor_shallow() { return SUCCESS; }
    Err dtor_deep() {
        Err err = ST_DEEP_DTOR_UNIMPL_ERR;
        debug_error(debug::category::data, errmsg(err), ' ', typeid(T).name());
        return err;
    }

    Err dtor();

    ~StateCRTP();
};

    //template<typename T = DefaultStateErrCategoryExt>
    //using StateErrCategory = typename StateCRTP<T>::ErrCategory;
    //
    //template<typename T = DefaultStateErrCategoryExt>
    //using StateErr = Err<StateErrCategory<T>>;

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

// call that in every init/constructor
template<typename T>
void StateCRTP<T>::state_init([[maybe_unused]] bool assignment) {
    assert(!state_flags.at(_initialized_bit) || assignment);
    state_flags.set(_initialized_bit);
}

template<typename T>
Err StateCRTP<T>::_dtor_shallow() {
    assert(!state_flags.at(_shallow_dtor_bit));

    auto err = static_cast<T*>(this)->dtor_shallow();

    if(err == SUCCESS)
        state_flags.set(_shallow_dtor_bit);

    return err;
}

template<typename T>
Err StateCRTP<T>::_dtor_deep() {
    assert(!state_flags.at(_deep_dtor_bit));

    //static_assert(meta::is_same_v<decltype(meta::declval<T>().dtor_deep()), Err >,
    //              "destory_deep() method has wrong return type.");
    auto err = static_cast<T*>(this)->dtor_deep();

    if(err == SUCCESS)
        state_flags.set(_deep_dtor_bit);

    return err;
}

template<typename T>
Err StateCRTP<T>::dtor() {
    Err err = SUCCESS;

#if defined(PROTO_DEBUG)
    if(state_flags.at(_deep_dtor_bit)) {
        err = ST_DBL_DEEP_DTOR_ERR;
        debug_error(debug::category::data, errmsg(err));
        return err; 
    }

    if(state_flags.at(_shallow_dtor_bit)) {
        err = ST_DBL_SHALLOW_DTOR_ERR;
        debug_error(debug::category::data, errmsg(err));
        return err;
    }
#endif

    if(state_flags.at(_initialized_bit)) {
        err = _dtor_shallow();
        if(!is_moved())
            err = _dtor_deep();
    } else {
        #if 1
        if constexpr(meta::is_base_of_v<debug::Marker, T>) {
            log_debug_marker(debug::category::main,
                             (*static_cast<T*>(this)));
        }
        #endif
        debug_warn(debug::category::main, errmsg(ST_DTOR_UNINIT_ERR));
        debug::stacktrace();
    }

    if(!err) state_flags.unset(_initialized_bit);
    return err;
}

template<typename T>
StateCRTP<T>::~StateCRTP() {
    if(state_flags.at(_defer_dtor_bit)){
        PROTO_DEPRECATED;

        if(dtor())
            debug_warn(debug::category::data,
                       "Deffered destructor failed for type ", typeid(T).name());
    }

#if defined(PROTO_DEBUG)
    if(state_flags.at(_initialized_bit) && !state_flags.at(_moved_bit)) {
        // if object was initialized, deep dtor has to be run on it at some point
        if(!state_flags.at(_deep_dtor_bit) && !state_flags.at(_no_dtor_bit)) {
            debug_warn(debug::category::data,
                       errmsg(ST_OUT_OF_SCOPE_LEAK_ERR), ' ', typeid(T).name());
            return;
        }
    }
#endif
}

} // namespace proto

