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


struct ECSTreeMemLayout {
    u64 size; 

    u64 comp_arrs_count;

    u64 header_sz;
    u64 header_sz_algn;

    u64 ent_arr_sz;
    u64 ent_arr_sz_algn;

    u64 comp_arrs_arr_sz;
    u64 comp_arrs_arr_sz_algn;

    CompBitset comp_bits;
    u64 comp_cnt[CompTList::size];

    u64 comp_arr_arena_sz_algn;
}; 

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

int calc_ecs_tree_memlayout(ECSTreeMemLayout& layout, Array<Entity>& ents);

int serialize_ecs_tree (MemBuffer buf, Array<Entity>& ents, ECSTreeMemLayout& layout);

int deserialize_ecs_tree (MemBuffer buf);


#if defined(PROTO_DEBUG)
static int debug_print(ECSTreeMemLayout * layout) {
    if(!layout) return -1;
    println("ECSTreeMemLayout { ");

    println("size: ", layout->size);
    println("header_sz: ", layout->header_sz);
    println("header_sz_algn: ", layout->header_sz_algn);
    println("ent_arr_sz: ", layout->ent_arr_sz);
    println("ent_arr_sz_algn: ", layout->ent_arr_sz_algn);
    println("comp_arrs_arr_sz: ", layout->comp_arrs_arr_sz);
    println("comp_arrs_arr_sz_algn: ", layout->comp_arrs_arr_sz_algn);

    print("comp_bits: ");
    for(u64 i=0; i<layout->comp_bits.bitsize; ++i) print((int)layout->comp_bits.at(i), ' ');
    println();

    print("comp_cnt: ");
    for(u64 i=0; i<count_of(layout->comp_cnt); ++i) print(layout->comp_cnt[i], ' ');
    println();

    println("comp_arr_arena_sz_algn: ", layout->comp_arr_arena_sz_algn);
    println("}");
    return 0;
}

static int debug_print(ECSTreeHeader * header) {
    if(!header) return -1;
    println("ECSTreeHeader { ");

    println("signature: ", header->signature);
    println("size: ", header->size);
    println("ents.offset: ", header->ents.offset);
    println("ents.count: ", header->ents.count);
    println("ents.size: ", header->ents.size);

    print("comp_bits: ");
    for(u64 i=0; i<header->comp_bits.bitsize; ++i) print((int)header->comp_bits.at(i), ' ');
    println();

    println("comp_arrs.offset: ", header->comp_arrs.offset);
    println("comp_arrs.count: ", header->comp_arrs.count);
    println("comp_arrs.size: ", header->comp_arrs.size);

    println("comp_arena_offset: ", header->comp_arena_offset);
    println("comp_arena_size: ", header->comp_arena_size);

    println("}");
    return 0;
}

#endif


} // namespace proto
 
