#pragma once
#include "proto/core/meta.hh"
#include "proto/core/StateCRTP.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/debug/markers.hh"
#include "proto/core/memory/common.hh"

namespace proto{

template<typename T, u64 _size,
         bool has_state = meta::is_base_of_v<StateCRTP<T>, T> >
struct StaticArray
    : debug::Marker {

    using DataType = T;

    // to allow for range-for
    struct Iterator {
        u64 _index;
        StaticArray<T, _size>& _array;

        Iterator(StaticArray<T, _size>& array, u64 index)
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
                other._array._data != _array._data ||
                other._array.size() != _array.size() ||
                other._index != _index;
        }
    };

    constexpr static u64 default_init_capacity = 0;
    T _data[size];

    template<typename ...U>
    StaticArray(U&&...ts) : _data{ meta::forward<U>(ts)... } {
        static_assert(sizeof...(ts) == sz);
    }


    u64 size() {
        return size;
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

    inline T * raw() {
        return _data;
    }

    template<typename U = T>
    inline auto contains(const U& element)
        -> meta::enable_if_t<meta::has_operator_eq_v<U>, bool> const
    {
        for(u64 i=0; i<_size; i++)
            if(_data[i] == element) return true;
        return false;
    }
};

} // namespace proto
