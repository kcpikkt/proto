#pragma once
#include "proto/core/meta.hh"
#include "proto/core/StateCRTP.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/debug/markers.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/util/Pair.hh"

namespace proto{

// NOTE(kcpikkt): Since it is based on two Arrays reallocation is unoptimal
//                Dont let Arrays internally manage memory. 
//                Instead Allocate for yourself contagious block for two arrays
//                and feed arrays their respective pointers or dont use them at all.
//               
template<typename Key, typename Value,
         bool key_has_state = meta::is_base_of_v<StateCRTP<T>, T>,
         bool value_has_state = meta::is_base_of_v<StateCRTP<T>, T>>
struct ArrayMap
    : StateCRTP<ArrayMap<Key, Value>>, debug::Marker {

    using State = StateCRTP<Array<T>>;

    // to allow for range-for
    struct Iterator {
        u64 _index;
        ArrayMap<Key, Value>& _map;

        Iterator(ArrayMap<Key, Value>& array, u64 index)
            :  _index(index), _map(map) {}

        Iterator& operator++() {
            _index++; return *this;
        }

        Iterator operator++(int) {
            Iterator ret = *this;
            _index++; return ret;
        }

        Pair<Key&, Value&> operator*(){
            return _array.pair_at_idx(_index);
        }

        bool operator!=(Iterator& other) {
            return other._index != _index                       ||
                   other._map.keys._data != _map.keys._data     ||
                   other._map.values._data != _map.values._data ;
        }
    };

    constexpr static u64 default_init_capacity = 0;
    Array<Key> keys;
    Array<Value> values;

    void _move(Array<Key, Value>&& other) {
        State::state_move(meta::forward<ArrayMap<Key, Value>>(other));
        key = meta::move(other.keys);
        values = meta::move(other.values);
    }

    ArrayMap() {} // noop, uninitialized state

    ArrayMap(ArrayMap<Key, Value>&& other) {
        _move(meta::forward<ArrayMap<Key, Value>>(other));
    }

    ArrayMap<Key, Value>& operator=(ArrayMap<Key, Value>&& other) {
        _move(meta::forward<ArrayMap<Key, Value>>(other));
        return *this;
    }

    //NOTE(kacper): no implicit copies; move or (N)RVO
    //              if copy is going to be needed, add named function
    ArrayMap(const ArrayMap<Key, Value>& other) = delete;
    ArrayMap<Key, Value>& operator=(const ArrayMap<Key, Value>& other) = delete;

    void init(u64 init_capacity, memory::Allocator * allocator) {
        State::state_init();
        assert(allocator);
        keys.init(init_capacity, allocator);
        values.init(init_capacity, allocator);
    }

    void init(memory::Allocator * allocator) {
        init(default_init_capacity, allocator);
    }

    void init_resize(u64 init_size, memory::Allocator * allocator) {
        init(init_size, allocator);
        resize(init_size);
    }

    u64 size() {
        return keys.size();
    }
    u64 capacity() {
        return keys.capacity();
    }

    inline Iterator begin() {
        return Iterator(*this, 0);
    }

    inline Iterator end() {
        return Iterator(*this, _size);
    }

    Value& back() {
        return at(_size - 1);
    }

    const Value& back() const {
        return at(_size - 1);
    }

    //////////////////////////////////////////// accessors
    inline Value* at(const Key& key) {
        for(u64 i=0; i<size(); ++i) 
            if(key == keys[i]) return values[i];
        return nullptr;
    }

    inline const Value* at(const Key& key) const {
        return at(key);
    }

    inline Value* operator[](const Key& key){
        return at(key);
    }

    inline Key& key_at_idx(u64 idx) {
        proto_assert(idx < size());
        return keys[idx];
    }

    inline const Key& key_at_idx(u64 idx) const {
        proto_assert(idx < size());
        return keys[idx];
    }

    inline Values& at_idx(u64 idx) {
        proto_assert(idx < size());
        return values[idx];
    }

    inline const Values& at_idx(u64 idx) const {
        proto_assert(idx < size());
        return values[idx];
    }

    inline Pair<Key&, Value&> pair_at_idx(u64 idx) {
        proto_assert(idx < size());
        return { keys[idx], values[idx] };
    }

    inline const Pair<Key&, Value&> pair_at_idx(u64 idx) const {
        return pair_at_idx(idx);
    }
    //////////////////////////////////////////// accessors

    //template<typename U = T,
    //         typename = decltype((T)meta::declval<U>())>
    //T& push_back(U&& element = T()) { // universal ref btw
    //    if(_size == _capacity) grow();
    //    assert(_size < _capacity);
    //    at(_size++) = meta::forward<U>(element);
    //    return back();
    //}

    //inline void zero() {
    //    assert(_data);
    //    // TODO(kacper): dont warn if T is trivially descructible
    //    if(_size != 0)
    //        debug_warn(proto::debug::category::memory, "zeroing non-empty array");
    //    memset(_data, 0, _capacity * sizeof(T));
    //}

    //inline T * raw() {
    //    return _data;
    //}

    //template<typename U = T>
    //inline auto contains(const U& element)
    //    -> meta::enable_if_t<meta::has_operator_eq_v<U>, bool> const
    //{
    //    for(u64 i=0; i<_size; i++)
    //        if(_data[i] == element) return true;
    //    return false;
    //}

    //inline void fill(const U& element)
    //{
    //    for(auto& e : *this) e = element;
    //}
    
    void grow() {
        reserve( proto::max(1, (2 * _size)) );
    }

    //void erase(u64 index) {
    //    at(index).~T();
    //    u64 i = index;
    //    for(; i<_size - 1; i++) {
    //        at(i) = meta::move(at(i + 1));
    //    } at(i).~T();
    //    _size--;
    //}


    void resize(u64 new_size) {
        keys.resize(new_size);
        values.resize(new_size);
        //if(new_size == _size) return;
        //if(new_size < _size) {
        //    if constexpr(has_state) {
        //        for(size_t i=new_size; i<_size; i++)
        //            (_data + i)->~T();
        //    }
        //} else {
        //    if(new_size > _capacity)
        //        reserve(new_size);

        //    for(size_t i=_size; i<new_size; i++)
        //          new ((void*)(_data + i)) T(); //FIXME(kacper):
        //}
        //_size = new_size;
    }

    void reserve(u64 new_capacity) {
        keys.reserve(new_capacity);
        values.reserve(new_capacity);
        //assert(_allocator);
        //if(!_allocator) debug::stacktrace();
        //if(new_capacity <= _capacity) return;

        //u64 bufsz = new_capacity * sizeof(T);
        //assert(bufsz);

        //_data = (_data)
        //    ? (T*)_allocator->realloc(_data, bufsz)
        //    : (T*)_allocator->alloc(bufsz);

        //assert(_data);
        //_capacity = new_capacity;
    }

    //Err<typename State::ErrCategory> destroy_shallow() {}

    Err<typename State::ErrCategory> destroy_deep() {
        keys.destroy();
        values.destroy();
    }
};

} // namespace proto
