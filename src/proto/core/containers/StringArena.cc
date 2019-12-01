#include "proto/core/containers/StringArena.hh"

#include "proto/core/util/StringView.hh"
#include "proto/core/string.hh"

namespace proto {
void StringArena::_move(StringArena&& other) {
    DataholderBase::dataholder_move(meta::forward<StringArena>(other));
    _data = other._data;
    _cursor = other._cursor;
    _capacity = other._capacity;
    _count = other._count;
    _allocator = other._allocator;
    _offsets = meta::move(other._offsets);
}

StringArena::StringArena() {} // noop, uninitialized state

StringArena::StringArena(StringArena&& other) {
    _move(meta::move(other));
}

StringArena& StringArena::operator=(StringArena&& other) {
    _move(meta::move(other));
    return *this;
}

void StringArena::init(memory::Allocator * allocator) {
    init(default_init_capacity, allocator);
}

void StringArena::init(u64 init_capacity, memory::Allocator * allocator) {
    DataholderBase::dataholder_init();
    assert(allocator);
    _allocator = allocator;

    reserve(init_capacity);
    _cursor = _data;
    _offsets.init(allocator);
}

void StringArena::init_split(StringView str, char delimiter,
                             memory::Allocator * allocator)
{
    // just don't, I don't want to check for it
    assert(str[0] != '\0');
    assert(str[str.length() - 1] != '\0');

    init(max(str.length() + 1, default_init_capacity), allocator);
    u32 count = strview_count(str, delimiter) + 1;
    _offsets.resize(count);
    strview_copy(_data, str);

    _cursor = _data + str.length() + 1;

    str_swap(_data, delimiter, '\0');

    u32 view_index = 0;
    _offsets[view_index++] = Offset(0, strlen(_data));

    for(u32 i=0; i<str.length(); i++) {
        if(_data[i] == '\0') {
            _offsets[view_index++] =
                Offset(i + 1, strlen(_data + i + 1));
        }
    }
}

u64 StringArena::count() {
    return _offsets.size();
}

StringView StringArena::operator[](u64 index) {
    return StringView(_data + _offsets[index].value,
                      _offsets[index].length);
}

void StringArena::reserve(u64 new_capacity) {
    assert(_cursor >= _data);
    assert((_data + _capacity) >= _cursor);

    assert(_allocator);
    if(new_capacity <= _capacity) return;

    u64 bufsz = new_capacity * sizeof(char);
    assert(bufsz);

    u64 cursor_offset = _cursor - _data;

    log_info(1,"reserved: ", bufsz);
    _data = (_data)
        ? (char*)_allocator->realloc(_data, bufsz)
        : (char*)_allocator->alloc(bufsz);

    _cursor = _data + cursor_offset;

    assert(_data);
    _capacity = new_capacity;
}
void StringArena::grow(u64 least_capacity) {
    reserve(2*least_capacity);
}

void StringArena::store(StringView str) {
    assert(_cursor >= _data);
    assert((_data + _capacity) >= _cursor);

    u64 left = (_data + _capacity) - _cursor;
    u64 len = str.length() + 1;
    if(left < len)
        grow(_capacity - left + len);

    strview_copy(_cursor, str);
    
    _offsets.push_back(Offset(_cursor - _data, len - 1));
    _cursor += len;
}

void StringArena::destroy_shallow() {}

void StringArena::destroy_deep() {
    assert(_data);
    assert(_allocator);
    _allocator->free(_data);
    _count = 0;
    _capacity = 0;
    _data = nullptr;
    _cursor = nullptr;
}

}
