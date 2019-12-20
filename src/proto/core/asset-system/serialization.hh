#pragma once
#include "proto/core/asset-system/common.hh"
#include "proto/core/context.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/util/algo.hh"
#include "proto/core/util/Buffer.hh"
#include "proto/core/platform/api.hh"
#include "proto/core/math/hash.hh"

namespace proto {
namespace serialization {

    constexpr static u64 asset_file_signature = 0x79677967;
    struct AssetFileHeader {
        u64 signature = asset_file_signature;

        u8 name[PROTO_ASSET_MAX_NAME_LEN];
        AssetHandle handle;
        u64 deps_offset;
        u64 deps_count;
        u64 deps_size;

        u64 data_offset;
    };

    struct AssetDependency {
        u8 name[PROTO_ASSET_MAX_NAME_LEN];
        AssetHandle handle;
    };

    template<typename T>
    void serialize_specific_asset_to_buffer(T*, MemBuffer buffer);

    template<typename T>
    MemBuffer serialize_asset(T* asset, memory::Allocator * allocator);

    template<>
    void serialize_specific_asset_to_buffer<Mesh>(Mesh* mesh, MemBuffer buffer);
 
    template<typename T>
    void deserialize_specific_asset_buffer(T * asset, MemBuffer buffer);

    //    template<>
    //    void deserialize_specific_asset_buffer<Mesh>(Mesh * mesh, MemBuffer buffer);

    void deserialize_specific_asset_buffer(AssetHandle handle, MemBuffer buffer);

    AssetHandle deserialize_asset_buffer(MemBuffer buffer);


    template<typename T>
    int save_asset(T * asset,
                   const char * path,
                   AssetContext * context = proto::context);

    int save_asset(AssetHandle handle,
                   const char * path,
                   AssetContext * context = proto::context);

    //struct _AssetFileSavelistEntry {
    //    char name[PROTO_ASSET_MAX_NAME_LEN];
    //    AssetHandle handle;
    //    //NOTE(kacper): You can store union of pointer to
    //    //              the actual asset types but idk.
    //    //NOTE(kacper): maybe in future if I would need to avoid
    //    //              double lookup
    //};

    using Savelist = Array<AssetHandle>;

    int create_asset_tree_savelist_rec (Savelist& savelist,
                                        AssetHandle handle);


    int save_asset_tree_rec(AssetHandle handle,
                            StringView dirpath,
                            AssetContext * asset_context = proto::context);
    
    AssetHandle load_asset_dir(StringView dirpath,
                               AssetContext * context = proto::context);
    AssetHandle load_asset(StringView path,
                           AssetContext * context = proto::context);
} // namespace serialization    
} // namespace proto

