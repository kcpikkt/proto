#pragma once
#include "proto/core/meta.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/memory/common.hh"

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
            constexpr static AssetTypeIndex index = INDEX;       \
            constexpr static const char * name = PROTO_STR(TYPE); \
        };
     
    PROTO_ASSET_TYPE(InvalidAsset, 0);
    PROTO_ASSET_TYPE(Mesh,         1);
    PROTO_ASSET_TYPE(Material,     2);
    PROTO_ASSET_TYPE(Texture2D,    3);
    PROTO_ASSET_TYPE(Cubemap,      4);

    struct AssetHandle {
        u32 hash = 0;
        using TypeIndex = AssetTypeIndex;
        TypeIndex type = AssetType<InvalidAsset>::index;

        bool operator==(AssetHandle other) const;
        bool is_valid() const;
        operator bool() const;
    };

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
                map_type_info<Mesh>();         break;
            case AssetType<Material>::index:
                map_type_info<Material>();     break;
            case AssetType<Texture2D>::index:
                map_type_info<Texture2D>();    break;
            case AssetType<Cubemap>::index:
                map_type_info<Cubemap>();      break;
            default:
                map_type_info<InvalidAsset>(); break;
            }
        }

        AssetType(AssetHandle handle)
            : AssetType(handle.type)
        {}
    };

    //template<typename T> struct AssetTypeId;

    constexpr static AssetHandle invalid_asset_handle = AssetHandle();

    struct Asset {
        AssetHandle handle;
    };

    struct InvalidAsset : Asset {};

    struct AssetMetadata {
        AssetHandle handle;
        char name[PROTO_ASSET_MAX_NAME_LEN];
        char filepath[PROTO_ASSET_MAX_PATH_LEN];
        bool is_cached = false;
        void * cached;
        Array<AssetHandle> deps;
        //        u32 reference_count;
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
}    

} // namespace proto
