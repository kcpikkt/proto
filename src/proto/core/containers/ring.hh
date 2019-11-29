#pragma once
#include "proto/core/common.hh"
#include <cstddef>
#include <type_traits>
#include <cassert>

template<typename T, size_t _bufsize>
class ring {
public:
    size_t size() {
        return _size;
    }
    size_t capacity() {
        return _bufsize;
    }
    T * data() {
        return _arr;
    }

    T& operator[] (size_t index) {
        assert(index < _size);
        return _at(index);
    }

    template<typename Integer,
             typename = std::enable_if_t< std::is_integral<Integer>::value>>
    T operator<<=(Integer n) {
        assert(n != 0);

        T ret = _at(n-1);
        shift_forward(n);
        return ret;
    }

    void shift_forward(size_t n = 1) {
        assert(_size >= n);
        _start = (_start + n) % _bufsize;
        _size -= n;
    }


    void push_back(T element) {
        assert(_size < _bufsize);
        _at(_size++) = element;
    }

private:
    size_t _start = 0;
    size_t _size = 0;

    T _arr[_bufsize];

    inline T& _at(size_t index) {
        assert(index < _size);
        return _arr[_index_map(index)];
    }

    inline size_t _index_map(size_t index){
        return (_start + (index % _size)) % _bufsize;
    }
};


