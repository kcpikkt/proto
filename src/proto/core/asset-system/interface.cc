#include "proto/core/asset-system/interface.hh"
#include "proto/core/math/hash.hh"

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

AssetHandle create_or_get_asset(StringView name,
                                       StringView filepath,
                                       AssetTypeIndex type)
{
    AssetHandle handle = make_handle(name, type);
    AssetMetadata * metadata = get_metadata(handle);
    return (metadata) ? handle : create_asset(name, filepath, type);
}
AssetHandle create_asset(StringView name,
                                StringView filepath,
                                AssetTypeIndex type)
{
    return create_asset(context, name, filepath, type);
}
AssetHandle create_asset(AssetContext * asset_context,
                                StringView name,
                                StringView filepath,
                                AssetTypeIndex type)
{
    AssetContext & ctx = *asset_context;
    AssetRegistry & reg = ctx.assets;

    DynamicArray<AssetMetadata> * insert_arr = nullptr;
    switch(type) {
    case AssetType<Mesh>::index:     { insert_arr = &reg.meshes;   } break;
    case AssetType<Material>::index: { insert_arr = &reg.materials;} break;
    case AssetType<Texture>::index:  { insert_arr = &reg.textures; } break;
    default:
        debug_warn(1, "requested asset of unsupported type");
    }

    if(insert_arr) {

        AssetHandle handle =  make_handle(name, type);

        if(get_metadata(handle))
            debug_warn(debug::category::data,
                       "Asset handle collision, name: ", name);

        insert_arr->push_back();
        AssetMetadata * metadata = &insert_arr->back(); 

        strview_copy(metadata->name, name);

        if(filepath)
            strview_copy(metadata->filepath, filepath);

        metadata->handle = handle;

        metadata->deps.init(30, &ctx.asset_metadata_allocator);
        
        switch(type) {
        case AssetType<Mesh>::index:     {
            ctx.meshes.push_back();

            Mesh * mesh = &ctx.meshes.back();
            assert(mesh);

            //// TODO(kacper): designated asset_memory
            mesh->init(&context->memory);

            mesh->handle = metadata->handle;
        } break;
        case AssetType<Material>::index: {
            ctx.materials.push_back();
            //// TODO(kacper): designated asset_memory
            //_asset_context->materials.back().init(&context->memory);

            Material * material = &ctx.materials.back();
            assert(material);

            material->handle = metadata->handle;

                   } break;
        case AssetType<Texture>::index:  {
            ctx.textures.push_back();

            Texture * texture = &ctx.textures.back();
            assert(texture);

            texture->init(&context->memory);

            texture->handle = metadata->handle;
        } break;
        default:
            debug_warn(1, "requested asset of unsupported type");
        }
                return metadata->handle;
    }
    return invalid_asset_handle;
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
    case AssetType<Texture>::index:  {
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

//void get_deps_rec(DynamicArray<AssetHandle> & depslist,
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

void get_deps_rec(DynamicArray<AssetHandle> & depslist,
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
    DynamicArray<T> * lookup_arr = nullptr;

    /*  */ if constexpr(meta::is_same_v<T, Mesh>) {
        lookup_arr =  &ctx.meshes;

    } else if constexpr (meta::is_same_v<T, Material>) {
        lookup_arr =  &ctx.materials;

    } else if constexpr (meta::is_same_v<T, Texture>) {
        lookup_arr =  &ctx.textures;

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

template Mesh *     get_asset<Mesh>(AssetHandle handle);
template Material * get_asset<Material>(AssetHandle handle);
template Texture *  get_asset<Texture>(AssetHandle handle);

AssetMetadata * get_metadata(AssetHandle handle) {
    return get_metadata(context, handle);
}

AssetMetadata * get_metadata(AssetContext * asset_context,
                             AssetHandle handle)
{
    AssetContext & ctx = *asset_context;
    AssetRegistry & reg = ctx.assets;

    DynamicArray<AssetMetadata> * lookup_arr = nullptr;
    switch(handle.type) {
    case AssetType<Mesh>::index:     { lookup_arr = &reg.meshes;    } break;
    case AssetType<Material>::index: { lookup_arr = &reg.materials; } break;
    case AssetType<Texture>::index:  { lookup_arr = &reg.textures;  } break;
    default:
        debug_warn(1, "requested metadata of asset of unsupported type");
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
