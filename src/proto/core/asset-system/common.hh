#pragma once
#include "proto/core/meta.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/math/hash.hh"

// this system do not know what it is yet, excuse it

namespace proto {
    struct AssetContext;

    #define PROTO_ASSET_MAX_NAME_LEN 32
    #define PROTO_ASSET_MAX_PATH_LEN 256

    struct InvalidAsset;
    struct Mesh;
    struct Texture2D;
    struct Cubemap;
    struct Material;
    struct ShaderProgram;

    using AssetTypeIndex = u8;

    template<u8 _index, const char * _name> struct _AssetType {
        constexpr static AssetTypeIndex index = _index;
        constexpr static const char * name = _name;
    };

    struct RuntimeAssetType {};
    template<typename = RuntimeAssetType> struct AssetType;

    #define PROTO_ASSET_TYPE(TYPE, INDEX)                         \
        template<> struct AssetType<TYPE> {                       \
            using type = TYPE;                                    \
            constexpr static AssetTypeIndex index = INDEX;        \
            constexpr static const char * name = PROTO_STR(TYPE); \
            constexpr static const u32 hash = hash::crc32( {name, sizeof(PROTO_STR(TYPE)) - 1} ); \
        };
     
    PROTO_ASSET_TYPE(InvalidAsset, 0);
    PROTO_ASSET_TYPE(Mesh,         1);
    PROTO_ASSET_TYPE(Material,     2);
    PROTO_ASSET_TYPE(Texture2D,    3);
    PROTO_ASSET_TYPE(Cubemap,      4);
    PROTO_ASSET_TYPE(ShaderProgram,5);

#pragma pack(push, 1)
    struct AssetHandle {
        u32 hash = 0;
        using TypeIndex = AssetTypeIndex;
        // 16 million index hint range is enough
        u32 idx_hint : 24, type : 8;

        static_assert(sizeof(TypeIndex) == 1);

        bool operator==(AssetHandle other) const;
        bool is_valid() const;
        operator bool() const;
    };
#pragma pack(pop)
    static_assert(sizeof(AssetHandle) == 8);

    // default for runtime typeinfo
    template<typename T>
    struct AssetType {
        const char * name;
        AssetTypeIndex index;
        
    private:
        template<typename U>
        void map_type_info(){
            name = AssetType<U>::name;
            index = AssetType<U>::index;
        }
    public:
        AssetType(AssetTypeIndex index){
            switch(index){
            case AssetType<Mesh>::index:
                map_type_info<Mesh>();          break;
            case AssetType<Material>::index:
                map_type_info<Material>();      break;
            case AssetType<Texture2D>::index:
                map_type_info<Texture2D>();     break;
            case AssetType<Cubemap>::index:
                map_type_info<Cubemap>();       break;
            case AssetType<ShaderProgram>::index:
                map_type_info<ShaderProgram>(); break;
            default:
                map_type_info<InvalidAsset>();  break;
            }
        }

        AssetType(AssetHandle handle)
            : AssetType(handle.type)
        {}
    };

    //template<typename T> struct AssetTypeId;

    constexpr static AssetHandle invalid_asset_handle = AssetHandle{};

    struct Asset {
        AssetHandle handle;
        bool operator==(const Asset& other) const {
            return handle == other.handle;
        }
    };

    struct InvalidAsset : Asset {};

    struct AssetMetadata {
        AssetHandle handle;
        char name[PROTO_ASSET_MAX_NAME_LEN];

        u32 archive_node_idx_hint;
        u32 archive_hash;

        Array<AssetHandle> deps;
    };

    //DEP
    using AssetId = size_t;

    struct AssetIdGenerator {
        inline static AssetId _id_count = 1; // let 0 be error id
        static AssetId next_id() {
            return (_id_count++);
        }
    };

namespace serialization {
    template<typename T> struct AssetHeader;
    template<typename T> inline u64 serialized_size(T&);
}    

} // namespace proto
