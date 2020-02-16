#pragma once
#include "proto/core/platform/File.hh"
#include "proto/core/util/Buffer.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/util/namespace-shorthands.hh"
#include "proto/core/error-handling.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/meta.hh"
#include "proto/core/entity-system/common.hh"

namespace proto {
namespace serialization {

u64 calc_flat_asset_archive_size(Array<AssetHandle>& assets);

#define PROTO_ARCHIVE_MAX_NAME_LEN 255
#define PROTO_ARCHIVE_MAX_PATH_LEN 255

struct ArchiveErrCategory : ErrCategoryCRTP<ArchiveErrCategory> {
    enum : u32 { success = 0,
                 out_of_memory,
                 invalid_archive,
                 file_open_fail,
                 file_write_fail,
                 file_read_fail,
                 file_resize_fail,
                 file_mapping_fail,
                 header_alloc_fail,
                 no_free_nodes,
                 asset_not_cached,
    };

    static ErrMessage message(ErrCode code) {
        switch(code) {
        case success:
            return "Success";
        case out_of_memory:
            return "Implement archive resize";
        case invalid_archive:
            return "The archive is corrupted.";
        case file_write_fail:
            return "Failed to write to archive file.";
        case file_read_fail:
            return "Failed to read archive file.";
        case file_resize_fail:
            return "Failed to resize archive file.";
        case file_mapping_fail:
            return "Failed to map archive file to memory.";
        case file_open_fail:
            return "Failed to open archive file.";
        case header_alloc_fail:
            return "Failed to allocate memory for archive file header cache.";
        case no_free_nodes:
            return "No free nodes left in the archive.";
            // perhaps last block of node table could be pointer to block of
            // more nodes? that would be kinda cool, no restriction on number of nodes
            // though it would be more tidious to implement
        case asset_not_cached:
            return "Could not obtain cached memory of an asset.";
        default:
            return "Unknown error.";
        }
    }
};

using ArchiveErr = Err<ArchiveErrCategory>;

struct Archive : StateCRTP<Archive> {
    
    struct Superblock {
        constexpr static u64 signature = 0x70726172;
        u64 sig = signature;
        u32 hash;

        u64 archive_size;
        u32 node_count;
        u32 free_node_count;

        u64 node_table_size;
        u64 node_table_offset;

        u64 data_size;
        u64 data_offset;
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

        u64 block_size;
        u64 block_offset;

        u64 parent_index;

        inline bool is_free() {return type == free;}

        Node() {}
    };

    // MemBuffer cached_header;
    Superblock * superblock;
    Array<Node> nodes;

    sys::File file;
    MemBuffer mapping;
    u64 cursor;

    u8 * _data_off2addr(u64 offset);

    u64 _addr2data_off(void * addr);

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
        //destroy(); // this is going to be just shallow destory
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

    ArchiveErr _commit_cached_header();

    StateErr<Archive> destroy_deep();

    ArchiveErr create(StringView filepath, u32 node_count, u64 data_size);
    ArchiveErr open(StringView filepath /* , sys::File::Mode filemode = sys::File::read_mode */ );
    ArchiveErr store(AssetHandle handle);
    ArchiveErr store(Array<Entity>&);

    MemBuffer get_node_memory(u64 node_idx);
    AssetHandle load_asset(u32 index);

    u64 _find_free_node();
};

} // namespace serialization
} // namespace proto
