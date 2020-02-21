#pragma once
#include "proto/core/entity-system/common.hh"
#include "proto/core/entity-system/components.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/util/Bitset.hh"
#include "proto/core/serialization/common.hh"

namespace proto {

// Idea is memcpy by default, specialize if you want something more sophisticated
template<typename T>
inline void serialize_comp(MemBuffer buf, T& comp) {
    static_assert(meta::is_base_of_v<Component, T>);
    assert(buf.size >= sizeof(T));

    memcpy(buf.data, &comp, sizeof(T));
}

constexpr static u64 ecs_tree_signature = 0x79677967;
struct ECSTreeHeader {
    u64 signature = ecs_tree_signature;

    u64 size;

    ArrayHeader ents;
    Bitset<CompTList::size> comp_bits;

    // this one is actually arrays of arrays of components
    // in the same order as they appear in comp_bits
    ArrayHeader comp_arrs;

    // in the arena is where they actually lie
    u64 comp_arena_offset;
    u64 comp_arena_size;
};

int create_ecs_tree_header
    (ECSTreeHeader& header, u64 (&comp_cnt)[CompTList::size], Array<Entity>& ents);

int serialize_ecs_tree
    (MemBuffer buf, Array<Entity>& ents, u64 (&comp_cnt)[CompTList::size], ECSTreeHeader& header);

int deserialize_ecs_tree (MemBuffer buf);


} // namespace proto
 
