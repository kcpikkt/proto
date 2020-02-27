#pragma once
#include "proto/core/platform/File.hh"
#include "proto/core/util/Bitset.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/util/namespace-shorthands.hh"
#include "proto/core/error-handling.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/entity-system/serialization.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/meta.hh"
#include "proto/core/entity-system/common.hh"

namespace proto {
    //namespace serialization {

u64 calc_flat_asset_archive_size(Array<AssetHandle>& assets);

#define PROTO_ARCHIVE_MAX_NAME_LEN 255
#define PROTO_ARCHIVE_MAX_PATH_LEN 255

    // TODO(kacper): perhaps non mapped header mode
struct Archive : StateCRTP<Archive> {
    
    constexpr static u64 ar_superblock_signature = 0x70726172;
    struct Superblock {
        u64 signature = ar_superblock_signature;
        u32 hash;

        // should to be the same (or at least less) as file size
        u64 archive_size;
        u64 header_sz;

        // bitmap telling us about arena block being or not being free
        u64 arena_bitmap_offset;
        u64 arena_bitmap_size;
        u64 arena_bitmap_bitsize;

        // bitmap telling us about node being or not being free
        u64 node_bitmap_offset;
        u64 node_bitmap_size;
        u64 node_bitmap_bitsize;

        // node table
        u64 node_table_offset;
        u64 node_table_count;
        u64 node_table_free;
        u64 node_table_size;

        // block size
        u64 block_size;

        // should be arena_bitmap_bitsize * block_size?
        u64 arena_cap;
        // actual filled size
        u64 arena_size;
        // superblock itself, bitmaps and node table comes before it
        u64 arena_offset;

        char name[PROTO_ARCHIVE_MAX_NAME_LEN];
    };

    struct Node {
        u32 hash;
        char name[255];

        AssetHandle handle;
        enum Type : u8 { free = 0,
                         directory,
                         asset,
                         ecs_tree
        } type;

        u64 blk_idx;
        u64 blk_cnt;
        u64 parent_index;

        inline bool is_free() {return type == free;}

        Node() {}
    };

    // MemBuffer cached_header;
    Superblock * sblk;
    Bitset<0> arena_bitmap;
    Bitset<0> node_bitmap;
    Array<Node> nodes;

    // superblock and nodes are memory mapped
    // so there is no need to commit any edits?
    MemBuffer mapping;
    sys::File file;

    constexpr static u64 _def_arena_cap = mem::gb(1);
    constexpr static u64 _def_block_size = mem::kb(1);
    static_assert(_def_arena_cap % _def_block_size == 0);

    constexpr static u64 _def_node_count = _def_arena_cap / _def_block_size;

    constexpr static u64 _def_init_arena_size = 0;

    Err create(StringView filepath,
               u64 node_cnt = _def_node_count,
               u64 arena_cap  = _def_arena_cap,
               u64 block_size = _def_block_size);

    Err open(StringView filepath ,
             sys::File::Mode filemode = sys::File::read_mode);

    Err store(AssetHandle handle);
    // You precalculated the layout? great!
    Err store(Array<Entity>&, ECSTreeMemLayout * const layout = nullptr);
 
    Err _node_sanity_check();

    u64 _alloc_node();
    Err _free_node(u64 idx);

    Err _arena_alloc(u64 node_idx, u64 bsize);

    void set_name(StringView name);

    Err dtor_deep();

    #if 0 
    u8 * _off2addr(u64 offset);
    u64  _addr2off(void * addr);

    inline u64 size() const {
        return superblock->archive_size; }

    inline u64 data_size() const {
        return superblock->data_size; }

    inline u64 node_count() const {
        return superblock->node_count; }

    void set_name(StringView name);

    void _move(Archive&& other) {
        State::state_move(meta::forward<Archive>(other));
        mapping = other.mapping;
        superblock = other.superblock;
        nodes = meta::move(other.nodes);
        file = other.file;
        //dtor(); // this is going to be just shallow destory
    }

    Archive() {} // noop, uninitialized state

    Archive(Archive&& other) {
        _move(meta::forward<Archive>(other));
    }

    Archive& operator=(Archive&& other) {
        _move(meta::forward<Archive>(other));
        return *this;
    }

    //NOTE(kacper): no implicit copies; move or (N)RVO
    //              if copy is going to be needed, add named function
    Archive(const Archive& other) = delete;
    Archive& operator=(const Archive& other) = delete;


    //Optional<u32> contains(AssetHandle asset) {
    //    for(u32 i=0; i<superblock->node_count; ++i) {
    //        if(nodes[i].type == Node::asset && nodes[i].hash == asset.hash) return i;
    //    }
    //    return {};
    //}

    // actually this function is probably rather copy data to specified buffer than expose it to user
    // so it can be e.g. copied to mapped mem without caching?
    // get separate function like copy_node_data()

    //MemBuffer get_node_data(u32 index) {
    //    assert(superblock->node_count < index);
    //    auto& node = node_table[index];

    //    if(node.type == Node::free) return {};

    //    return MemBuffer{ _offset2addr(node.block_offset), node.block_size };
    //}

    //MemBuffer get_asset_data(AssetHandle asset) {
    //    if(auto idx_opt = contains(asset)) {
    //        return get_node_data(idx_opt.value);
    //    }
    //    return {};
    //}

    // perhaps you can just map file to memory, right?

    Err _commit_cached_header();

    Err dtor_deep();

    Err create(StringView filepath, u32 node_count, u64 data_size);
    Err open(StringView filepath /* , sys::File::Mode filemode = sys::File::read_mode */ );
    Err store(AssetHandle handle);

    Err store(Array<Entity>&, ECSTreeMemLayout * const layout = nullptr);

    MemBuffer get_node_memory(u64 node_idx);
    AssetHandle load_asset(u32 index);

    u64 _find_free_node();
    #endif
};

    //} // namespace serialization
} // namespace proto
