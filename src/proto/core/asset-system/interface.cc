#include "proto/core/asset-system/interface.hh"
#include "proto/core/math/hash.hh"
#include "proto/core/graphics/Texture2D.hh"
#include "proto/core/graphics/Cubemap.hh"
#include "proto/core/graphics/Mesh.hh"
#include "proto/core/graphics/ShaderProgram.hh"
#include "proto/core/containers/ArrayMap.hh"

namespace proto {

AssetHandle make_handle(StringView name, AssetTypeIndex type){
    // TODO(kacper): salt hash depending on type
    return AssetHandle{.hash = hash::crc32(name),
                       .type = type };
}

    //void add_dependency(AssetMetadata * dependant, AssetHandle dependency)
    //{
    //    assert(dependant->handle.is_valid());
    //    assert(dependency.is_valid());
    //    dependant->deps.push_back(dependency);
    //}

// specialize if necessary
//template<typename T> static void _default_asset_init(T& asset) {
//    //asset.init();
//}

    //template<> void _default_asset_init<Mesh>(Mesh& mesh) {
    //    mesh.init(&context->memory);
    //}
    //
    //template<> void _default_asset_init<Material>(Material&) {}

template<typename T>
static ArrayMap<AssetHandle, Pair<T, AssetMetadata>> * _get_asset_map(AssetContext * asset_context) {
    /**/ if constexpr (meta::is_same_v<T, Mesh>)
        return &asset_context->meshes;
    else if constexpr (meta::is_same_v<T, Material>)
        return &asset_context->materials;
    else if constexpr (meta::is_same_v<T, Texture2D>)
        return &asset_context->textures;
    else if constexpr (meta::is_same_v<T, Cubemap>)
        return &asset_context->cubemaps;
    else if constexpr (meta::is_same_v<T, ShaderProgram>)
        return &asset_context->shader_programs;

    debug_error(debug::category::data,
                "Requested asset array map of unsupported asset type");
    assert(0); return nullptr;
}

template<typename T>
static Pair<T, AssetMetadata> * _get_pair(AssetHandle& handle, AssetContext * asset_context) {
    if(AssetType<T>::index != AssetType(handle.type).index) return nullptr;

    // cannot fail
    ArrayMap<AssetHandle, Pair<T, AssetMetadata>> * map = _get_asset_map<T>(asset_context);
    proto_assert(map);

    Pair<T, AssetMetadata>* pair = nullptr;
    if(handle.idx_hint < map->size() && map->key_at_idx(handle.idx_hint) == handle) {
        pair = &map->at_idx(handle.idx_hint);
    } else {
        u32 index = map->idx_of(handle);

        if(index != map->size()) {
            // index hinting for the next time
            handle.idx_hint = (u32)index;
            return &map->at_idx(index);
        }
    }
    return pair;
}

template<typename T, typename Ret>
Ret create_asset(StringView name, AssetContext * asset_context) {
    auto & ctx = *asset_context;

    ArrayMap<AssetHandle, Pair<T, AssetMetadata>> * map = _get_asset_map<T>(&ctx);
    proto_assert(map);

    AssetHandle handle = make_handle<T>(name); // TODO(kcpikkt): handle hash generation should also depend on type

    if(_get_pair<T>(handle, asset_context))
        debug_warn(debug::category::data,
                   "Asset handle collision: ", name, ", ", AssetType<T>::name);

    handle.idx_hint = map->size();
    Pair<T, AssetMetadata> & pair = map->push_back(handle);

    auto& asset = pair.first;
    auto& metadata = pair.second;

    strview_copy(metadata.name, name);
    metadata.handle = handle;

    metadata.deps.init(0, &ctx.asset_metadata_allocator);

    asset.handle = handle;

    /*  */ if constexpr(meta::is_same_v<Ret, AssetHandle>) {
        return handle;
    } else if constexpr(meta::is_same_v<Ret, T*>) {
        return &asset;
    } else if constexpr(meta::is_same_v<Ret, T&>) {
        return  asset;
    } else if constexpr(meta::is_same_v<Ret, AssetMetadata*>) {
        return &metadata;
    } else if constexpr(meta::is_same_v<Ret, AssetMetadata&>) {
        return  metadata;
    }
}


AssetHandle create_asset(StringView name, AssetTypeIndex type, AssetContext * asset_context) {
    switch(type) {
    case AssetType<Mesh>::index:
        return create_asset <Mesh, AssetHandle>(name, asset_context);

    case AssetType<Material>::index:
        return create_asset <Material, AssetHandle>(name, asset_context);

    case AssetType<Texture2D>::index:
        return create_asset <Texture2D, AssetHandle>(name, asset_context);

    case AssetType<Cubemap>::index:
        return create_asset <Cubemap, AssetHandle>(name, asset_context);

    case AssetType<ShaderProgram>::index:
        return create_asset <ShaderProgram, AssetHandle>(name, asset_context);
    }

    debug_error(debug::category::data,
                "Cannot create asset of type ", AssetType(type).name);
    return invalid_asset_handle;
}

//void destroy_asset(AssetContext * asset_context,
//                          AssetHandle handle)
//{
//    AssetContext & ctx = *asset_context;
//    switch(handle.type) {
//    case AssetType<Mesh>::index:     {
//        for(u32 i=0; i<ctx.meshes.size(); i++) {
//            if(ctx.meshes[i].handle == handle) {
//                ctx.meshes[i].destroy();
//                ctx.meshes.erase(i);
//                break;
//            }
//        }
//
//        for(u32 i=0; i<ctx.assets.meshes.size(); i++) {
//            if(ctx.assets.meshes[i].handle == handle) {
//                //ctx.assets.meshes[i].destroy();
//                ctx.assets.meshes.erase(i);
//                break;
//            }
//        }
//     
//    } break;
//    case AssetType<Material>::index: {
//        PROTO_NOT_IMPLEMENTED;
//    } break;
//    case AssetType<Texture2D>::index:  {
//        //Texture * texture = get_asset<Texture>(handle);
//        PROTO_NOT_IMPLEMENTED;
//    } break;
//    default:
//        debug_warn(1, "requested metadata of asset of unsupported type");
//    }
//}
//
//void destroy_asset(AssetHandle handle){
//    return destroy_asset(context, handle);
//}

//void get_deps_rec(Array<AssetHandle> & depslist,
//                  AssetHandle handle)
//{
//    assert(handle); //TODO(kacper): err msg, softfail
//    AssetMetadata * metadata = get_metadata(handle);
//    assert(metadata); //TODO(kacper): err msg, softfail
//
//    int ret;
//
//    for(u32 i=0; i<metadata->deps.size(); i++) {
//        //TODO(kacper): do not ignore recursive ret value?
//        create_asset_tree_savelist_rec(savelist, metadata->deps[i]);
//    }
//
//    // NOTE(kacper): super-asset before or after its dependencies, hmm...
//    if(!savelist.contains(handle))
//        savelist.push_back(handle);

//}

//void get_deps_rec(Array<AssetHandle> & depslist,
//                  AssetHandle handle)
//{
//    assert(handle); //TODO(kacper): err msg, softfail
//    AssetMetadata * metadata = get_metadata(handle);
//    assert(metadata); //TODO(kacper): err msg, softfail
//
//    for(u32 i=0; i<metadata->deps.size(); i++) {
//        //TODO(kacper): do not ignore recursive ret value?
//        get_deps_rec(depslist, metadata->deps[i]);
//    }
//
//    // NOTE(kacper): super-asset before or after its dependencies, hmm...
//    if(!depslist.contains(handle))
//        depslist.push_back(handle);
//}
template<typename T>
T * get_asset(AssetHandle& handle, AssetContext * asset_context) {
    Pair<T, AssetMetadata> * pair = _get_pair<T>(handle, asset_context);
    static_assert(meta::is_base_of_v<Asset, T>);

    return pair ? &pair->first : nullptr; 
}

template<typename T>
static AssetMetadata * _get_metadata(AssetHandle& handle, AssetContext * asset_context) {
    Pair<T, AssetMetadata> * pair = _get_pair<T>(handle, asset_context);
   
    return pair ? &pair->second : nullptr; 
}

AssetMetadata * get_metadata(AssetHandle& handle, AssetContext * asset_context)
{
    switch(handle.type) {
    case AssetType<Mesh>::index:          return _get_metadata<Mesh>(handle, asset_context);
    case AssetType<Material>::index:      return _get_metadata<Material>(handle, asset_context);
    case AssetType<Texture2D>::index:     return _get_metadata<Texture2D>(handle, asset_context);
    case AssetType<Cubemap>::index:       return _get_metadata<Cubemap>(handle, asset_context);
    case AssetType<ShaderProgram>::index: return _get_metadata<ShaderProgram>(handle, asset_context);
    default:
        debug_warn(debug::category::data,
                   "Requested metadata of asset of unsupported type ", AssetType(handle.type).name);
        return nullptr;
    }

}

template<typename T>
T & get_asset_ref(AssetHandle& handle, AssetContext * asset_context) {
    T * ret = get_asset<T>(handle, asset_context);
    proto_assert(ret);
    return *ret;
}

#define INSTANTIATE_CREATE_ASSET_FUNCTIONS_FOR(T) \
template AssetHandle create_asset<T, AssetHandle> (StringView, AssetContext*); \
template T * create_asset<T, T *> (StringView, AssetContext*); \
template T & create_asset<T, T &> (StringView, AssetContext*); \
template AssetMetadata * create_asset<T, AssetMetadata *> (StringView, AssetContext*); \
template AssetMetadata & create_asset<T, AssetMetadata &> (StringView, AssetContext*); \
template T* get_asset<T>(AssetHandle&, AssetContext*);                    \
template T& get_asset_ref<T>(AssetHandle&, AssetContext*);

INSTANTIATE_CREATE_ASSET_FUNCTIONS_FOR(Mesh);
    //INSTANTIATE_CREATE_ASSET_FUNCTIONS_FOR(Material);
    //INSTANTIATE_CREATE_ASSET_FUNCTIONS_FOR(Texture2D);
    //INSTANTIATE_CREATE_ASSET_FUNCTIONS_FOR(Cubemap);
INSTANTIATE_CREATE_ASSET_FUNCTIONS_FOR(ShaderProgram);


} // namespace proto
