#pragma once

#include "proto/core/common/types.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/util/algo.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/DataholderCRTP.hh"
#include "proto/core/string.hh"

namespace proto {

struct StringArena : DataholderCRTP<StringArena> {
    using DataholderBase = DataholderCRTP<StringArena>;

    constexpr static u64 default_init_capacity = 32;

    u64 _count;
    u64 _capacity;
    char * _data;
    char * _cursor;
    memory::Allocator * _allocator;

    Array<StringView> _views;

   void _move(StringArena&& other) {
        DataholderBase::dataholder_move(meta::forward<StringArena>(other));
        _data = other._data;
        _cursor = other._cursor;
        _capacity = other._capacity;
        _count = other._count;
        _allocator = other._allocator;
        _views = meta::move(other._views);
    }

    StringArena() {} // noop, uninitialized state

    StringArena(StringArena&& other) {
        _move(meta::forward<StringArena>(other));
    }

    StringArena& operator=(StringArena&& other) {
        _move(meta::forward<StringArena>(other));
        return *this;
    }

    //NOTE(kacper): no implicit copies; move or (N)RVO
    //              if copy is going to be needed, add named function
    StringArena(const StringArena& other) = delete;
    StringArena& operator=(const StringArena& other) = delete;


    void init(memory::Allocator * allocator) {
        init(default_init_capacity, allocator);
    }

    void init(u64 init_capacity, memory::Allocator * allocator) {
        DataholderBase::dataholder_init();
        assert(allocator);
        _allocator = allocator;
        reserve(init_capacity);
        _cursor = _data;
        _views.init(allocator);
    }

    void init_split(StringView str, char delimiter, memory::Allocator * allocator) {
        //u32 count = (strview_count(str, delimiter) + 1);
        //init(max(str.length(), default_init_capacity), allocator);
        //vardump(count);
        //_views.reserve(10);
    }

    u64 count() {
        return _views.size();
    }

    StringView operator[](u64 index) {
        return _views[index];
    }
    
    void reserve(u64 new_capacity) {
        assert(_allocator);
        if(new_capacity <= _capacity) return;

        u64 bufsz = new_capacity * sizeof(u8);
        assert(bufsz);

        _data = (_data)
            ? (char*)_allocator->realloc(_data, bufsz)
            : (char*)_allocator->alloc(bufsz);

        assert(_data);
        _capacity = new_capacity;
    }
    void grow(u64 least_capacity) {
        reserve(2*least_capacity);
    }

    void store(StringView str) {
        assert((_data + _capacity) >= _cursor);
        u64 left = (_data + _capacity) - _cursor;
        u64 len = str.length() + 1;
        if(left > len)
            grow(_capacity - left + len);

        strview_copy(_cursor, str);

        _views.push_back(StringView(_cursor, str.length()));
    }

    void destroy_shallow() {}

    void destroy_deep() {
        assert(_data);
        assert(_allocator);
        _allocator->free(_data);
        _count = 0;
        _capacity = 0;
    }
};

} // namespace proto

