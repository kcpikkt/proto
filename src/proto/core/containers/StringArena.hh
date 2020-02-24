#pragma once

#include "proto/core/common/types.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/StateCRTP.hh"
#include "proto/core/util/Bitfield.hh"
#include "proto/core/debug/markers.hh"

namespace proto {

struct StringArena : StateCRTP<StringArena>, debug::Marker{
    using StateBase = StateCRTP<StringArena>;

    // to allow for range-for
    struct Iterator {
        u64 _index;
        StringArena& _arena;

        Iterator(StringArena& arena, u64 index)
            : _index(index), _arena(arena) {}

        Iterator& operator++() {
            _index++; return *this;
        }

        Iterator operator++(int) {
            Iterator ret = *this;
            _index++; return ret;
        }

        StringView operator*(){
            return _arena[_index];
        }

        bool operator!=(Iterator& other) {
            return //FIXME?(kacper):
                other._arena._data != _arena._data || other._index != _index;
        }
    };

    struct Offset {
        u64 length;
        u32 value;
        Bitfield<u8> flags;
        static constexpr u8 free_bit = 0;

        Offset() {}

        Offset(decltype(value) val, decltype(length) len)
            : length(len), value(val) {}

        void _copy(const Offset& other) {
            memcpy(this, &other, sizeof(Offset)); //idk
        }
        Offset(const Offset& other) { _copy(other); }
        Offset& operator=(const Offset& other) { _copy(other); return *this; }
    };

    constexpr static u64 default_init_capacity = 32;

    u64 _count = 0;
    u64 _capacity = 0;
    char * _data = nullptr;
    char * _cursor = nullptr;
    memory::Allocator * _allocator = nullptr;

    Array<Offset> _offsets;

    void _move(StringArena&& other);

    StringArena(); // noop, uninitialized state

    StringArena(StringArena&& other);
    StringArena& operator=(StringArena&& other);

    //NOTE(kacper): no implicit copies; move or (N)RVO
    //              if copy is going to be needed, add named function
    StringArena(const StringArena& other) = delete;
    StringArena& operator=(const StringArena& other) = delete;

    void init(memory::Allocator * allocator);

    void init(u64 init_capacity, memory::Allocator * allocator);

    void init_split(StringView str, char delimiter, memory::Allocator * allocator);

    u64 count();

    inline Iterator begin() {
        return Iterator(*this, 0);
    }

    inline Iterator end() {
        return Iterator(*this, count());
    }

    StringView operator[](u64 index);
    
    void reserve(u64 new_capacity);
    void grow(u64 least_capacity);

    void store(StringView str);

    bool contains(StringView str);

    Err dtor_deep();
};

} // namespace proto

