#pragma once
#include "proto/core/memory.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/meta.hh"

namespace proto {
    template<typename T>
    struct DynamicArray {
        using DataType = T;

        size_t _capacity = 0;
        size_t _size = 0;
        memory::Allocator * _allocator;
        bool is_initialized = false; // idk, perhaps delete it

        T * _data = nullptr;

        inline size_t size() {
            return _size;
        }

        inline size_t capacity() {
            return _capacity;
        }

        // why do I have to do that precisely?
        template<typename U = DataType>
        inline auto contains(const U& element)
            -> meta::enable_if_t<meta::has_operator_eq<U>::value, bool> const
        {
            for(u32 i=0; i<size(); i++)
                if(element == _data[i]) return true;
            return false;
        }
        //inline auto contains(const T& element)
        //    -> meta::enable_if_t<meta::has_operator_eq_v<T>, bool> const
        //{
        //    for(u32 i=0; i<size(); i++)
        //        if(element == _data[i]) return true;
        //    return false;
        //}



        //inline T * front() {
        //    assert(_size > 0);
        //    return & _data[0];
        //}

        //inline T * back() {
        //    assert(_size > 0);
        //    return & _data[_size - 1];
        //}
        inline void zero() {
            assert(_data);
            // TODO(kacper): dont warn if T is trivially descructable
            if(_size != 0)
                debug_warn(proto::debug::category::memory,
                           "zeroing non-empty array");

            memset(_data, 0, _capacity * sizeof(T));
        }

        inline T * raw() {
            return _data;
        }

        ~DynamicArray() {
            //if(is_initialized && _data) {
            //    assert(_allocator);
            //    _allocator->free(_data);
            //}
        }

        int init(memory::Allocator * allocator) {
            assert(!is_initialized);
            assert(_size == 0);
            assert(_data == nullptr);

            assert(allocator);
            _allocator = allocator;

            _capacity = 0;
            is_initialized = true;
            return 0;
        }

        int init(size_t init_capacity, memory::Allocator * allocator) {
            assert(!is_initialized);
            assert(_size == 0);
            assert(_data == nullptr);
            assert(allocator);
            _allocator = allocator;

            if(init_capacity) {
                _data = (T*)allocator->alloc(init_capacity * sizeof(T));

                if(_data) {
                    _capacity = init_capacity;
                    is_initialized = true;
                    return 0;
                } else {
                    assert(0);
                    return -1;
                }
            } else {
                _capacity = 0;
                is_initialized = true;
                return 0;
            }
        }
        int destroy() {
            if(_data) {
                for(size_t i=0; i<_size; i++) (_data + i)->~T();
                assert(_allocator);
                _allocator->free(_data);
                _data = nullptr;
                _allocator = nullptr;
                _size = 0;
                _capacity = 0;
            }
            return 0;
        }

        int init_resize(size_t init_size,
                        memory::Allocator * allocator,
                        T element = T())
        {
            int ret = init(init_size, allocator);
            if(!ret) resize(init_size, element);
            return ret;
        }

        size_t push_back(T * element) {
            assert(element);
            if(_size < _capacity) {
                new ((void*)(_data + _size++)) T(*element);
            } else {
                assert(_size == _capacity);
                //TODO: 
                reserve( max(10, (size_t)(_capacity * 1.5f)) );
                new ((void*)(_data + _size++)) T(*element);
            }
            return _size - 1;
        }

        size_t push_back(T element = T()) {
            if(_size < _capacity) {
                new ((void*)(_data + _size++)) T(element);
            } else {
                assert(_size == _capacity);
                reserve( max(10, (size_t)(_capacity * 1.5f)) );
                new ((void*)(_data + _size++)) T(element);
            }
            return _size - 1;
        }

        void reserve(size_t new_capacity) {
            if(new_capacity <= _capacity) return;

            assert(_allocator);

            _data = (_data)
                ? (T*)_allocator->realloc(_data, new_capacity * sizeof(T))
                : (T*)_allocator->alloc(new_capacity * sizeof(T));

            assert(_data);
            _capacity = new_capacity;
        }

        void resize(size_t new_size, T element = T()) {
            assert(is_initialized);
            if(new_size == _size) return;
            if(new_size < _size) {
                for(size_t i=new_size; i<_size; i++)
                    (_data + i)->~T();
                _size = new_size;
            } else {
                if(new_size > _capacity)
                    reserve(new_size);

                for(size_t i=_size; i<new_size; i++)
                    new ((void*)(_data + i)) T(element);

                _size = new_size;
            }
        }

        inline void erase(size_t index) {
            assert(index < _size);
            (_data + index)->~T();
            memmove((void*)(_data + index),
                    (void*)(_data + index + 1),
                    (_size - (index + 1)) * sizeof(T));
            _size--;
        }
        
        T& operator[] (size_t index) {
            assert(is_initialized);
            assert_info(index < _size,
                        proto::debug::category::main,
                        index, " < ", _size);

            return *(_data + index);
        }

        inline T& front() {
            assert(_size > 0);
            return *_data;
        }

        inline T& back() {
            assert(_size > 0);
            return *(_data + _size - 1);
        }
    };
}

