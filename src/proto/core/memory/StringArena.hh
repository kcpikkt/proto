#pragma once
#if 0
#include "proto/core/debug.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/common/types.hh"

namespace proto {

    //struct StaticStringArena {
    //    Array<StringView> _views;
    //    const char * _data = nullptr;
    //    u64 _size = 0;
    //    memory::Allocator * _allocator;
    //
    //    int init(StringView str,
    //             char delimiter,
    //             memory::Allocator * allocator)
    //    {
    //        assert(allocator);
    //        _allocator = allocator;
    //
    //        u32 views_size = 1;
    //        //counting strings
    //        for(u64 i=0; i<str.length(); i++) {
    //            if(str[i] == delimiter) views_size++;
    //        }
    //
    //        _views.init_resize(views_size, allocator);
    //
    //        _data = str.str();
    //        _size = str.length();
    //
    //        const char * cursor = _data;
    //        u64 view_index = 0;
    //        for(u64 i=0; i<str.length(); i++) {
    //            if(str[i] == delimiter) {
    //                u64 len = (_data + str.length()) - cursor;
    //
    //                _views[view_index] = StringView(cursor, len);
    //                cursor = _data + str.length();
    //            }
    //        }
    //
    //        return 0;
    //    }
    //
    //    StringView operator[](u64 index) {
    //        return _views[index];
    //    }
    //};
////TODO(kacper): add reusing strings
struct StringArena {
    Array<StringView> _views;
    char * _data = nullptr;
    const char * _cursor = nullptr;
    u64 _size = 0;
    u64 _capacity = 0;
    memory::Allocator * _allocator;
    float grow_coeff = 2.0f;


    int init(u64 init_strlen_cap,
             memory::Allocator * allocator) {
        assert(allocator);
        _allocator = allocator;

        _views.init(init_strlen_cap/10, allocator);

        _data = (char*)_allocator->alloc(init_strlen_cap);
        assert(_data);
        _capacity = init_strlen_cap;
        _cursor = _data;
        _size = 0;

        return 0;
    }

    int init(memory::Allocator * allocator) {
        return init(0, allocator);
    }

    
    int init(StringView str,
             char delimiter,
             memory::Allocator * allocator)
    {
        assert(allocator);
        _allocator = allocator;
    
        u32 views_size = 1;
        //counting strings
        for(u64 i=0; i<str.length(); i++) {
            if(str[i] == delimiter) views_size++;
        }
    
        _views.init_resize(views_size, allocator);
    
        _data = (char*)_allocator->alloc(str.length());
        assert(_data);
        memcpy(_data, str.str(), str.length());

        _capacity = str.length();
        _size = str.length();
    
        const char * tmp_cursor = _data;
        u64 view_index = 0;
        for(u64 i=0; i<str.length(); i++) {
            if(str[i] == delimiter) {
                u64 len = (_data + i) - tmp_cursor;
    
                _views[view_index++] = StringView(tmp_cursor, len);
                i++;
                tmp_cursor = _data + i;
            }
        } 
        //last one

        _views[view_index] = StringView(tmp_cursor,
                                        (_data + str.length()) - tmp_cursor);

        assert(view_index == views_size - 1);
    
        return 0;
    }
    
 
    // StringView argument just for convenience,
    // string length is calculated in const char * -> StringView conversion
    // (or does not have to if user passes StringView)
    StringView alloc(StringView str) {
        //PROTO_NOT_IMPLEMENTED;
        assert(_data);
        assert(_cursor);
        u64 left = _cursor - (_data + _capacity);
        if(left < str.length()) {
            u64 new_cap = (u64)((_capacity - left + str.length()) * grow_coeff);
            reserve(new_cap);
        }
        left = _cursor - (_data + _capacity);
        assert(left > str.length());
        memcpy((void*)_cursor, (void*)str.str(), str.length());

        _views.push_back(StringView(_cursor, str.length()));
        _cursor += str.length();
        _size += str.length();

        return _views.back();
    }

    u64 count(){
        return _views.size();
    }

    void reserve(size_t new_capacity) {
        assert(_allocator);
        _data = (char*)_allocator->realloc(_data, new_capacity);
        assert(_data);
        _capacity = new_capacity;
    }

    StringView operator[](u64 index) {
        return _views[index];
    }

};


} //namespace proto



#endif
