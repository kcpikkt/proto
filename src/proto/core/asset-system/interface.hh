#pragma once
#include "proto/core/common.hh"
#include "proto/core/context.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/containers/Array.hh"
#include "proto/core/meta.hh"

namespace proto {
    struct AssetContext;

    // TODO(kacper): think maybe about shared pointers?

    AssetHandle make_handle(StringView name, AssetTypeIndex type);

    template<typename T>
    inline AssetHandle make_handle(StringView name){
        return make_handle(name, AssetType<T>::index);
    }

    // NOTE(kacper): no need for header definition,
    //               all specializations are explicitly instantiated
    template<typename T>
    T * get_asset(AssetHandle& handle,
                  AssetContext * asset_context = proto::context);

    // no fail, crash if null
    // same as above but returns reference
    template<typename T>
    T & get_asset_ref(AssetHandle& handle,
                      AssetContext * asset_context = proto::context);

    AssetMetadata * get_metadata(AssetHandle& handle,
                                 AssetContext * asset_context = proto::context);

    // TODO(kcpikkt): add meta::decay<T>&& = T() parameter
    // but first implement decay
    // NOTE(kcpikkt): later: wait, but why exacly? dude, don't leave such unexplainatory comments!
    template<typename T, typename Ret = AssetHandle>
    Ret create_asset(StringView name,
                     AssetContext * asset_context = proto::context);

    template<typename T>
    inline T& create_asset_rref(StringView name,
                                AssetContext * asset_context = proto::context)
    {
        return create_asset<T, T&>(name, asset_context);
    }

    AssetHandle create_asset(StringView name, AssetTypeIndex type,
                             AssetContext * asset_context = proto::context);



    #define INVOKE_FTEMPL_WITH_ASSET(ftempl, accessor, handle)          \
    [&](){                                                                          \
        switch(handle.type) {                                                       \
        case AssetType<Mesh>::index:      return ftempl(accessor<Mesh>(handle)); \
        case AssetType<Texture2D>::index: return ftempl(accessor<Texture2D>(handle)); \
        case AssetType<Material>::index:  return ftempl(accessor<Material>(handle)); \
        default: assert(0);                                                         \
        }                                                                           \
    }()

    #define INVOKE_FTEMPL_WITH_ASSET_REF(ftempl, handle) \
        INVOKE_FTEMPL_WITH_ASSET(ftempl, get_asset_ref, handle)

    #define INVOKE_FTEMPL_WITH_ASSET_PTR(ftempl, handle) \
        INVOKE_FTEMPL_WITH_ASSET(ftempl, get_asset, handle)


    MemBuffer get_asset_cached(AssetHandle handle);

    template<typename T>
    static MemBuffer get_asset_cached(AssetHandle handle) {
        assert(AssetType<T>::index == handle.type);

        if(auto asset = get_asset<T>(handle))
            return asset->cached;

        return {};
    }

    template<>
    MemBuffer get_asset_cached<Material>(AssetHandle handle) {
        assert(AssetType<Material>::index == handle.type);

        if(auto mat = get_asset<Material>(handle))
            return { {mat}, sizeof(Material)};

        return {};
    }






     //#define INVOKE_FTEMPL_WITH_ASSET_H(function_templ, handle)       \
    //[&](){                                                                          \
    //    switch(handle.type) {                                                       \
    //    case AssetType<Mesh>::index:      return function_templ<Mesh>(handle);      \
    //    case AssetType<Texture2D>::index: return function_templ<Texture2D>(handle); \
     //    case AssetType<Material>::index:  return function_templ<Material>(handle); \
    //    default: assert(0);                                                         \
    //    }                                                                           \
    //}()
    //void destroy_asset(AssetHandle handle); 
    //void destroy_asset(AssetContext * asset_context,
    //                   AssetHandle handle); 

    //void get_deps_rec(Array<AssetHandle> & depslist,
    //                  AssetHandle handle);

    //void add_dependency(AssetMetadata * dependant,
    //                    AssetHandle dependency);

    
} // namespace proto
