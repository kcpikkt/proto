#pragma once
#include "proto/core/meta.hh"
#include "proto/core/StateCRTP.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/debug/markers.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/util/Optional.hh"
#include "proto/core/util/FunctionView.hh"

namespace proto{

template<typename T,
         bool has_state = meta::has_state_v<T> >
struct Array
    : StateCRTP<Array<T>>, debug::Marker {

    using DataType = T;
    using State = StateCRTP<Array<T>>;

    // to allow for range-for
    struct Iterator {
        u64 _index;
        Array<T>& _array;

        Iterator(Array<T>& array, u64 index)
            :  _index(index), _array(array) {}

        Iterator& operator++() {
            _index++; return *this;
        }

        Iterator operator++(int) {
            Iterator ret = *this;
            _index++; return ret;
        }

        T& operator*(){
            return _array.at(_index);
        }

        bool operator!=(Iterator& other) {
            return //FIXME?(kacper):
                other._array._data != _array._data || other._index != _index;
        }
    };

    constexpr static u64 default_init_capacity = 0;
    T * _data = nullptr;
    u64 _count = 0;
    u64 _capacity = 0;
    memory::Allocator * _allocator = nullptr;

    void _move(Array<T>&& other) {
        State::state_move(meta::forward<Array<T>>(other));
        _data = other._data; other._data = nullptr;
        _allocator = other._allocator; other._allocator = 0;
        _count = other._count; other._count = 0;
        _capacity = other._capacity; other._capacity = 0;
    }

    Array() {} // noop, uninitialized state

    Array(Array<T>&& other) {
        _move(meta::forward<Array<T>>(other));
    }

    Array<T>& operator=(Array<T>&& other) {
        _move(meta::forward<Array<T>>(other));
        return *this;
    }

    //NOTE(kacper): no implicit copies; move or (N)RVO
    //              if copy is going to be needed, add named function
    Array(const Array<T>& other) = delete;
    Array<T>& operator=(const Array<T>& other) = delete;

    void init(u64 init_capacity, memory::Allocator * allocator) {
        State::state_init();
        assert(allocator);
        _allocator = allocator;

        reserve(init_capacity);
    }

    void init(memory::Allocator * allocator) {
        init(default_init_capacity, allocator);
    }

    void init_resize(u64 init_count, memory::Allocator * allocator) {
        init(init_count, allocator);
        resize(init_count);
    }

    // use responsibly 
    void init_place(void * data, u64 capacity) {
        State::state_init();
        State::state_flags.set(State::_moved_bit); // to prevent destruction
        assert(data);
        _data = (T*)data;
        _capacity = capacity;
    }

    inline void init_place_resize(void * data, u64 count) {
        init_place(data, count);
        resize(count);
    }

    // it is being a bit unconsistent to call it size
    // where it is rather count, change it perhaps
    u64 size() const {
        return _count;
    }
    // ok, lets have count for now and see how it goes
    u64 count() const {
        return _count;
    }

    u64 capacity() const {
        return _capacity;
    }

    inline Iterator begin() {
        return Iterator(*this, 0);
    }

    inline Iterator end() {
        return Iterator(*this, _count);
    }

    T& back() {
        return at(_count - 1);
    }

    const T& back() const {
        return at(_count - 1);
    }

    inline T& operator[](u64 index){
        return at(index);
    }

    inline T& at(u64 index) {
        proto_assert(index < _count);
        return _data[index];
    }

    inline const T& at(u64 index) const {
        assert(index < _count);
        return _data[index];
    }

    inline s64 idx_of(const T& val) const {
        u64 i=0;
        for(; i<size(); ++i)
            if(at(i) == val) break;
        return i;
    }

    template<typename U = T,
             typename = decltype((T)meta::declval<U>())>
    T& push_back(U&& element = T()) { // universal ref btw
        if(_count == _capacity) grow();
        assert(_count < _capacity);
        new (&at(_count++)) T(meta::forward<U>(element));
        return back();
    }

    inline void zero() {
        assert(_data);
        // TODO(kacper): dont warn if T is trivially descructible
        if(_count != 0)
            debug_warn(proto::debug::category::memory,
                        "zeroing non-empty array");
        memset(_data, 0, _capacity * sizeof(T));
    }

    inline T * raw() {
        return _data;
    }

    template<typename U = T>
    inline auto contains(const U& element)
        -> meta::enable_if_t<meta::has_operator_eq_v<U>, Optional<u64>> const
    {
        for(u64 i=0; i<_count; i++)
            if(_data[i] == element) return i;

        return {};
    }

    //inline void fill(const U& element)
    //{
    //    for(auto& e : *this) e = element;
    //}
    
    void grow() {
        reserve(proto::max(1, (2 * _count) ));
    }

    void erase(u64 index) {
        //at(index).~T();
        u64 i = index;
        for(; i<_count - 1; i++) {
            at(i) = meta::move(at(i + 1));
        } //at(i).~T();
        _count--;
    }

    u64 find_if(FunctionView<bool(T&)> predecate, u64 hint = 0) {
        if(hint && hint < _count && predecate(at(hint))) return hint;

        u64 i=0;
        for(; i<_count; i++) if(predecate(at(i))) break;
        return i;
    }

    u64 count_if(FunctionView<bool(T&)> predecate) {
        u64 acc = 0;
        for(auto e : *this) acc += predecate(e) ? 1 : 0;
        return acc;
    }

    void for_each(FunctionView<void(T&)> proc) {
        for(u64 i=0; i<_count; ++i) proc(_data[i]);
    }

    //void for_each(FunctionView<void(T&, u64)> proc) {
    //    for(u64 i=0; i<_count; ++i) proc(_data[i], i);
    //}


    void resize(u64 new_count) {
        if(new_count == _count) return;
        if(new_count < _count) {
            if constexpr(has_state) {
                for(size_t i=new_count; i<_count; i++)
                    (_data + i)->dtor();
            }
        } else {
            if(new_count > _capacity)
                reserve(new_count);

            //for(size_t i=_count; i<new_count; i++)
            //      new ((void*)(_data + i)) T();
            // NOTE(kacper): ok the thing is that i want to default contruct them by deafult
            // in case when I init_place default values can wipe my data
            // say AssetHandle is constructed as invalid_asset_handle by default to prevent mistakes
            // And I do like that behaviour but when I obtain data from archive and init_place array
            // as Archive.nodes default construction would then wipe my data
        }
        _count = new_count;
    }

    void reserve(u64 new_capacity) {
        assert(_allocator);
        if(!_allocator) debug::stacktrace();
        if(new_capacity <= _capacity) return;

        u64 bufsz = new_capacity * sizeof(T);
        assert(bufsz);

        _data = (_data)
            ? (T*)_allocator->realloc(_data, bufsz)
            : (T*)_allocator->alloc(bufsz);

        assert(_data);
        _capacity = new_capacity;
    }

    //Err<typename State::ErrCategory> dtor_shallow() {}

    Err<typename State::ErrCategory> dtor_deep() {
        assert(_allocator);
        auto err = State::ErrCategory::success;
        if constexpr(has_state) {
                // monitor for destruction errors
            for(u64 i=0; i<_count; i++) _data[i].dtor();
        }
        if(_data)
            _allocator->free(_data);
        _count = 0;
        _capacity = 0;
        return err;
    }
};

} // namespace proto
