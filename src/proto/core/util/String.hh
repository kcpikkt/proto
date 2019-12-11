#pragma once
#include "proto/core/common.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/DataholderCRTP.hh"
#include "proto/core/debug/assert.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/string.hh"

namespace proto {
    
    // NOT NECESSARILY NULL TERMINATED!
    struct String : Array<char> {

        void init(StringView view, memory::Allocator * allocator) {
            init_resize(view.length() + 1, allocator);
            strview_copy(_data, view);
        }
        
        StringView view() {
            return StringView(_data, strlen(_data)); 
        }

        operator bool() { return _data; }

        operator StringView() {
            return view();
        }
    };
} // namespace proto
