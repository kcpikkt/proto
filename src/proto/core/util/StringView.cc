#include "proto/core/util/StringView.hh"
#include "proto/core/string.hh"
    
namespace proto {
using Iterator = StringView::Iterator;

Iterator& Iterator::operator++() {
    _index++; return *this;
}

Iterator Iterator::operator++(int) {
    Iterator ret = *this;
    _index++; return ret;
}

char Iterator::operator*(){
    return _view[_index];
}

bool Iterator::operator!=(Iterator& other) {
    return 
        other._view.str != _view.str || other._index != _index;
}

 Iterator StringView::begin() {
    return Iterator(*this, 0);
}

 Iterator StringView::end() {
    return Iterator(*this, length);
}


bool StringView::is_cstring() {
    return (strlen(str) == length);
}

char StringView::operator[](u64 index) {
    assert(str);
    proto_assert(index < length);
    return str[index];
}

 StringView StringView::trim_prefix(u64 n) {
    assert(n <= length);
    length -= n; str += n;
    return StringView(str - n, n);
}

 StringView StringView::trim_suffix(u64 n) {
    assert(n <= length);
    length -= n;
    return StringView(str + length, n);
}

 StringView StringView::prefix(u64 n) {
    assert(n <= length);
    return StringView(str, n);
}

StringView StringView::suffix(u64 n) {
    assert(n <= length);
    return StringView(str + length - n, n);
}

bool operator==(StringView a, StringView b) {
    return strview_cmp(a, b);
}

} // namespace proto

