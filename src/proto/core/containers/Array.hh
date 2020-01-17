#pragma once
#include "proto/core/meta.hh"
#include "proto/core/StateCRTP.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/debug/markers.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/util/Optional.hh"

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
    u64 _size = 0;
    u64 _capacity = 0;
    memory::Allocator * _allocator = nullptr;

    void _move(Array<T>&& other) {
        State::state_move(meta::forward<Array<T>>(other));
        _data = other._data; other._data = nullptr;
        _allocator = other._allocator; other._allocator = 0;
        _size = other._size; other._size = 0;
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

    void init_resize(u64 init_size, memory::Allocator * allocator) {
        init(init_size, allocator);
        resize(init_size);
    }

    // use responsibly 
    void init_place(void * data, u64 capacity) {
        State::state_init();
        State::state_flags.set(State::_moved_bit); // to prevent destruction
        assert(data);
        _data = (T*)data;
        _capacity = capacity;
    }

    inline void init_place_resize(void * data, u64 capacity) {
        init_place(data, capacity);
        resize(capacity);
    }

    u64 size() const {
        return _size;
    }
    u64 capacity() const {
        return _capacity;
    }

    inline Iterator begin() {
        return Iterator(*this, 0);
    }

    inline Iterator end() {
        return Iterator(*this, _size);
    }

    T& back() {
        return at(_size - 1);
    }

    const T& back() const {
        return at(_size - 1);
    }

    inline T& operator[](u64 index){
        return at(index);
    }

    inline T& at(u64 index) {
        proto_assert(index < _size);
        return _data[index];
    }

    inline const T& at(u64 index) const {
        assert(index < _size);
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
        if(_size == _capacity) grow();
        assert(_size < _capacity);
        new (&at(_size++)) T(meta::forward<U>(element));
        return back();
    }

    inline void zero() {
        assert(_data);
        // TODO(kacper): dont warn if T is trivially descructible
        if(_size != 0)
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
        for(u64 i=0; i<_size; i++)
            if(_data[i] == element) return i;

        return {};
    }

    //inline void fill(const U& element)
    //{
    //    for(auto& e : *this) e = element;
    //}
    
    void grow() {
        reserve(proto::max(1, (2 * _size) ));
    }

    void erase(u64 index) {
        //at(index).~T();
        u64 i = index;
        for(; i<_size - 1; i++) {
            at(i) = meta::move(at(i + 1));
        } //at(i).~T();
        _size--;
    }

    u64 find( bool (*predecate)(T&)) {
        u64 i=0;
        for(; i<_size; i++) if(predecate(at(i))) break;
        return i;
    }


    void resize(u64 new_size) {
        if(new_size == _size) return;
        if(new_size < _size) {
            if constexpr(has_state) {
                for(size_t i=new_size; i<_size; i++)
                    (_data + i)->~T();
            }
        } else {
            if(new_size > _capacity)
                reserve(new_size);

            for(size_t i=_size; i<new_size; i++)
                  new ((void*)(_data + i)) T(); //FIXME(kacper):
        }
        _size = new_size;
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

    //Err<typename State::ErrCategory> destroy_shallow() {}

    Err<typename State::ErrCategory> destroy_deep() {
        assert(_allocator);
        auto err = State::ErrCategory::success;
        if constexpr(has_state) {
                // monitor for destruction errors
            for(u64 i=0; i<_size; i++) _data[i].destroy();
        }
        if(_data)
            _allocator->free(_data);
        _size = 0;
        _capacity = 0;
        return err;
    }
};

} // namespace proto
