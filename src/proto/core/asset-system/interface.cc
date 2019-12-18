#include "proto/core/asset-system/interface.hh"
#include "proto/core/math/hash.hh"
#include "proto/core/graphics/Texture2D.hh"
#include "proto/core/graphics/Cubemap.hh"
#include "proto/core/graphics/Mesh.hh"
#include "proto/core/graphics/ShaderProgram.hh"

namespace proto {

AssetHandle make_handle(StringView name, AssetTypeIndex type){
    // TODO(kacper): salt hash depending on type
    return AssetHandle{.hash = hash::crc32(name),
                       .type = type };
}

void add_dependency(AssetMetadata * dependant,
                    AssetHandle dependency)
{
    assert(dependant->handle.is_valid());
    assert(dependency.is_valid());
    dependant->deps.push_back(dependency);
}

// specialize if necessary
template<typename T> static void _default_asset_init(T& asset) {
    asset.init();
}

template<> void _default_asset_init<Mesh>(Mesh& mesh) {
    mesh.init(&context->memory);
}

template<> void _default_asset_init<Material>(Material&) {}

template<typename T>
static Array<T>* _get_asset_array() {
    /**/ if constexpr (meta::is_same_v<T, Mesh>)
        return &context->meshes;
    else if constexpr (meta::is_same_v<T, Material>)
        return &context->materials;
    else if constexpr (meta::is_same_v<T, Texture2D>)
        return &context->textures;
    else if constexpr (meta::is_same_v<T, Cubemap>)
        return &context->cubemaps;
    else if constexpr (meta::is_same_v<T, ShaderProgram>)
        return &context->shader_programs;

    debug_error(debug::category::data,
                "Requested asset array of unsupported asset type");
    assert(0); return nullptr;
}

template<typename T>
static Array<AssetMetadata>* _get_metadata_array()
{
    AssetRegistry& reg = context->assets;
    /**/ if constexpr (meta::is_same_v<T, Mesh>)
        return &reg.meshes;
    else if constexpr (meta::is_same_v<T, Material>)
        return &reg.materials;
    else if constexpr (meta::is_same_v<T, Texture2D>)
        return &reg.textures;
    else if constexpr (meta::is_same_v<T, Cubemap>)
        return &reg.cubemaps;
    else if constexpr (meta::is_same_v<T, ShaderProgram>)
        return &reg.shader_programs;

    debug_error(debug::category::data,
                "Requested metadata array of unsupported asset type");
    assert(0); return nullptr;
}

template<typename T, typename Ret, bool init>
Ret create_asset(StringView name) {
    auto & ctx = *context;

    Array<AssetMetadata> * insert_arr = _get_metadata_array<T>();
    AssetHandle handle =  make_handle<T>(name);

    if(get_metadata(handle))
        debug_warn(debug::category::data,
                   "Asset handle collision: ", name, ", ", AssetType<T>::name);

    AssetMetadata & metadata = insert_arr->push_back();

    strview_copy(metadata.name, name);
    metadata.handle = handle;
    //TODO(kacper): default zero
    metadata.deps.init(10, &ctx.asset_metadata_allocator);

    T& _asset = _get_asset_array<T>()->push_back();
    _asset.handle = handle;

    if constexpr(init)
        _default_asset_init<T>(_asset);
    
    /*  */ if constexpr(meta::is_same_v<Ret, AssetHandle>) {
        return handle;
    } else if constexpr(meta::is_same_v<Ret, T*>)  {
        return &_asset;
    } else if constexpr(meta::is_same_v<Ret, T&>)  {
        return  _asset;
    } else if constexpr(meta::is_same_v<Ret, AssetMetadata*>) {
        return &metadata;
    } else if constexpr(meta::is_same_v<Ret, AssetMetadata&>) {
        return  metadata;
    } else if constexpr(meta::is_same_v<Ret, AssetHandlePair<T>>) {
        return AssetHandlePair<T>{ handle, _asset};
    }
}

// silly part here, thank god for macros
#define INSTANTIATE_CREATE_ASSET_FUNCTIONS_FOR(T) \
template T * create_init_asset<T, T *>   (StringView); \
template T * create_asset<T, T *, false> (StringView); \
template T & create_init_asset<T, T &>   (StringView); \
template T & create_asset<T, T &, false> (StringView); \
template AssetHandle create_init_asset<T, AssetHandle>   (StringView); \
template AssetHandle create_asset<T, AssetHandle, false> (StringView); \
template AssetMetadata * create_init_asset<T, AssetMetadata *>   (StringView); \
template AssetMetadata * create_asset<T, AssetMetadata *, false> (StringView); \
template AssetMetadata & create_init_asset<T, AssetMetadata &>   (StringView); \
template AssetMetadata & create_asset<T, AssetMetadata &, false> (StringView); \
template AssetHandlePair<T>                                            \
   create_init_asset<T, AssetHandlePair<T>>  (StringView);            \
template AssetHandlePair<T>                                            \
   create_asset<T, AssetHandlePair<T>, false> (StringView);    \

INSTANTIATE_CREATE_ASSET_FUNCTIONS_FOR(Mesh);
INSTANTIATE_CREATE_ASSET_FUNCTIONS_FOR(Material);
INSTANTIATE_CREATE_ASSET_FUNCTIONS_FOR(Texture2D);
INSTANTIATE_CREATE_ASSET_FUNCTIONS_FOR(Cubemap);
INSTANTIATE_CREATE_ASSET_FUNCTIONS_FOR(ShaderProgram);

template<typename T, typename Ret>
Ret create_init_asset(StringView name) {
    return create_asset<T, Ret, true>(name);
}

template<bool init>
AssetHandle create_asset(StringView name, AssetTypeIndex type) {
    switch(type) {
    case AssetType<Mesh>::index:
        return create_asset <Mesh, AssetHandle, init>(name);

    case AssetType<Material>::index:
        return create_asset <Material, AssetHandle, init>(name);

    case AssetType<Texture2D>::index:
        return create_asset <Texture2D, AssetHandle, init>(name);

    case AssetType<Cubemap>::index:
        return create_asset <Cubemap, AssetHandle, init>(name);

    case AssetType<ShaderProgram>::index:
        return create_asset <ShaderProgram, AssetHandle, init>(name);
    }

    debug_error(debug::category::data,
                "Cannot create asset of type ", AssetType(type).name);
    return invalid_asset_handle;
}

template AssetHandle create_asset<true> (StringView, AssetTypeIndex);
template AssetHandle create_asset<false>(StringView, AssetTypeIndex);

AssetHandle create_init_asset(StringView name, AssetTypeIndex type) {
    return create_asset<true>(name, type);
}

void destroy_asset(AssetContext * asset_context,
                          AssetHandle handle)
{
    AssetContext & ctx = *asset_context;
    switch(handle.type) {
    case AssetType<Mesh>::index:     {
        for(u32 i=0; i<ctx.meshes.size(); i++) {
            if(ctx.meshes[i].handle == handle) {
                ctx.meshes[i].destroy();
                ctx.meshes.erase(i);
                break;
            }
        }

        for(u32 i=0; i<ctx.assets.meshes.size(); i++) {
            if(ctx.assets.meshes[i].handle == handle) {
                //ctx.assets.meshes[i].destroy();
                ctx.assets.meshes.erase(i);
                break;
            }
        }
     
    } break;
    case AssetType<Material>::index: {
        PROTO_NOT_IMPLEMENTED;
    } break;
    case AssetType<Texture2D>::index:  {
        //Texture * texture = get_asset<Texture>(handle);
        PROTO_NOT_IMPLEMENTED;
    } break;
    default:
        debug_warn(1, "requested metadata of asset of unsupported type");
    }
}

void destroy_asset(AssetHandle handle){
    return destroy_asset(context, handle);
}

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

void get_deps_rec(Array<AssetHandle> & depslist,
                  AssetHandle handle)
{
    assert(handle); //TODO(kacper): err msg, softfail
    AssetMetadata * metadata = get_metadata(handle);
    assert(metadata); //TODO(kacper): err msg, softfail

    for(u32 i=0; i<metadata->deps.size(); i++) {
        //TODO(kacper): do not ignore recursive ret value?
        get_deps_rec(depslist, metadata->deps[i]);
    }

    // NOTE(kacper): super-asset before or after its dependencies, hmm...
    if(!depslist.contains(handle))
        depslist.push_back(handle);
}



template<typename T>
T * get_asset(AssetHandle handle) {
    return get_asset<T>(context, handle);
}

template<typename T>
T * get_asset(AssetContext * asset_context,
              AssetHandle handle) {
    AssetContext & ctx = *asset_context;

    static_assert(meta::is_base_of_v<Asset, T>);
    Array<T> * lookup_arr = nullptr;

    /*  */ if constexpr(meta::is_same_v<T, Mesh>) {
        lookup_arr =  &ctx.meshes;

    } else if constexpr (meta::is_same_v<T, Material>) {
        lookup_arr =  &ctx.materials;

    } else if constexpr (meta::is_same_v<T, TextureInterface>) {
        /**/ if(handle.type == AssetType<Texture2D>::index)
            return get_asset<Texture2D>(&ctx, handle);
        else if(handle.type == AssetType<Cubemap>::index)
            return get_asset<Cubemap>(&ctx, handle);
        else
            lookup_arr = nullptr;
    } else if constexpr (meta::is_same_v<T, Texture2D>) {
        lookup_arr =  &ctx.textures;

    } else if constexpr (meta::is_same_v<T, Cubemap>) {
        lookup_arr =  &ctx.cubemaps;

    } else if constexpr (meta::is_same_v<T, ShaderProgram>) {
        lookup_arr =  &ctx.shader_programs;

    } else {
        assert(0 && "this should not even be instantiatied");
    }

    if(lookup_arr) {
        for(u32 i = 0; i<(*lookup_arr).size(); i++) {
            auto& asset = (*lookup_arr)[i];
            if(asset.handle == handle) return &asset;
        }
    }
    return nullptr;
}

template<typename T>
T * get_asset_must(AssetHandle handle) {
    T * ret = get_asset<T>(handle);
    assert(ret); return ret;
}

template<typename T>
T & get_asset_ref(AssetHandle handle) {
    return *get_asset_must<T>(handle);
}

template Mesh &             get_asset_ref<Mesh>             (AssetHandle handle);
template Material &         get_asset_ref<Material>         (AssetHandle handle);
template Texture2D &        get_asset_ref<Texture2D>        (AssetHandle handle);
template Cubemap &          get_asset_ref<Cubemap>          (AssetHandle handle);
template TextureInterface & get_asset_ref<TextureInterface> (AssetHandle handle);
template ShaderProgram &    get_asset_ref<ShaderProgram>    (AssetHandle handle);

AssetMetadata * get_metadata(AssetHandle handle) {
    return get_metadata(context, handle);
}

AssetMetadata * get_metadata(AssetContext * asset_context,
                             AssetHandle handle)
{
    AssetContext & ctx = *asset_context;
    AssetRegistry & reg = ctx.assets;

    Array<AssetMetadata> * lookup_arr = nullptr;
    switch(handle.type) {
    case AssetType<Mesh>::index:     { lookup_arr = &reg.meshes;    } break;
    case AssetType<Material>::index: { lookup_arr = &reg.materials; } break;
    case AssetType<Texture2D>::index:{ lookup_arr = &reg.textures;  } break;
    case AssetType<Cubemap>::index:  { lookup_arr = &reg.cubemaps;  } break;
    case AssetType<ShaderProgram>::index:
        { lookup_arr = &reg.shader_programs;  } break;
    default:
        debug_warn(1, "requested metadata of asset of unsupported type ",
                   AssetType(handle.type).name);
    }
    if(lookup_arr) {
        for(u32 i = 0; i<(*lookup_arr).size(); i++) {
            auto& metadata = (*lookup_arr)[i];
            if(metadata.handle == handle) return &metadata;
        }
    }
    return nullptr;
}
} // namespace proto
