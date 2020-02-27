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
    using AssetTList = meta::typelist<InvalidAsset, Mesh, Texture2D, Cubemap, Material, ShaderProgram>;

    template<u8 _index, const char * _name> struct _AssetType {
        constexpr static AssetTypeIndex index = _index;
        constexpr static const char * name = _name;
    };

    struct RuntimeAssetType {};
    template<typename = RuntimeAssetType> struct AssetType;

    #define PROTO_ASSET_TYPE(TYPE)                               \
        template<> struct AssetType<TYPE> {                             \
            using type = TYPE;                                          \
            constexpr static AssetTypeIndex index = AssetTList::index_of<TYPE>::value; \
            constexpr static const char * name = PROTO_STR(TYPE);       \
            constexpr static const u32 hash = hash::crc32( {name, sizeof(PROTO_STR(TYPE)) - 1} ); \
        };
     
    PROTO_ASSET_TYPE(InvalidAsset);
    PROTO_ASSET_TYPE(Mesh);
    PROTO_ASSET_TYPE(Material);
    PROTO_ASSET_TYPE(Texture2D);
    PROTO_ASSET_TYPE(Cubemap);
    PROTO_ASSET_TYPE(ShaderProgram);

    static_assert(AssetTList::size < 256);

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

    // Runtype type information lookup table for O(1) comptypeinfo
    struct _AssetTypeData {
        template<size_t I>
        using AssetAt = typename AssetTList::template at_t<I>;

        struct Data {
            const char * name;
            AssetTypeIndex index;
        };

        template<typename...> struct _Lookup;
        template<size_t...Is> struct _Lookup<meta::sequence<Is...>> {
            template<size_t I> constexpr static Data make_data() {
                return { AssetType<AssetAt<I>>::name, AssetType<AssetAt<I>>::index };
            }

            static_assert(sizeof...(Is) == AssetTList::size);
            inline static Data table[sizeof...(Is)] = { make_data<Is>()...};
        };
        using Lookup = _Lookup<meta::make_sequence<0,AssetTList::size>>;

        static constexpr Data& at(u64 i) {
            return Lookup::table[i];
        }
    };

    // default for runtime typeinfo
    template<typename T>
    struct AssetType {
        u8 idx;

        constexpr auto name() {
            return _AssetTypeData::at(idx).name; }

        constexpr auto index() {
            return _AssetTypeData::at(idx).index; }

        AssetType(AssetHandle handle) : idx(handle.type) {}
        AssetType(u8 type) : idx(type) {}
    };

    //template<typename T> struct AssetTypeId;

    constexpr static AssetHandle invalid_asset_handle = AssetHandle{};

    struct Asset {
        AssetHandle handle;
        MemBuffer cached;
        enum {
              on_gpu_bit = 0,
              cached_bit,
              archived_bit,
              asset_free_flags_bit
        };
        Bitset<8> flags;

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

    template<typename T> inline void serialize(AssetHeader<T>&, T&);
    template<typename T> inline void deserialize(T&, AssetHeader<T>&);
}    

} // namespace proto
