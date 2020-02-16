#pragma once
#include "proto/core/entity-system/common.hh"
#include "proto/core/entity-system/components.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/util/Bitset.hh"

namespace proto {

// Idea is memcpy by default, specialize if you want something more sophisticated
template<typename T>
inline void serialize_comp(MemBuffer buf, T& comp) {
    static_assert(meta::is_base_of_v<Component, T>);
    assert(buf.size >= sizeof(T));

    memcpy(buf.data, &comp, sizeof(T));
}

struct ECS_Tree_Header {
    struct ArrayHeader {
        u64 offset, size, count;
    };

    ArrayHeader entities;
    Bitset<CompTList::size> comp_bits;

    // this one is actually arrays of arrays of components
    // in the order as they appear in comp_bits
    ArrayHeader comp_headers;
};

} // namespace proto
