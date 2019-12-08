#pragma once
#include "proto/core/common.hh"
#include "proto/core/context.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/containers/Array.hh"

namespace proto {
    struct AssetContext;

    // TODO(kacper): think maybe about shared pointers?

    AssetHandle make_handle(StringView name, AssetTypeIndex type);

    // NOTE(kacper): no need for header definition,
    //               all specializations are explicitly instantiated
    template<typename T>
    T * get_asset(AssetHandle handle);
    template<typename T>
    T * get_asset(AssetContext * asset_context,
                  AssetHandle handle);

    AssetMetadata * get_metadata(AssetHandle handle);
    AssetMetadata * get_metadata(AssetContext * asset_context,
                                 AssetHandle handle);
    // just proxies
    template<typename T>
    AssetMetadata * get_metadata(T & asset) {
        return get_metadata(asset.handle);
    }

    template<typename T>
    AssetMetadata * get_metadata(T * asset){
        return get_metadata(asset->handle);
    }



    AssetHandle create_or_get_asset(StringView name,
                                    StringView filepath,
                                    AssetTypeIndex type);
    AssetHandle create_asset(StringView name,
                             StringView filepath,
                             AssetTypeIndex type); 
    AssetHandle create_asset(AssetContext * asset_context,
                             StringView name,
                             StringView filepath,
                             AssetTypeIndex type); 

    void destroy_asset(AssetHandle handle); 
    void destroy_asset(AssetContext * asset_context,
                       AssetHandle handle); 

    void get_deps_rec(Array<AssetHandle> & depslist,
                      AssetHandle handle);

    void add_dependency(AssetMetadata * dependant,
                        AssetHandle dependency);

    
} // namespace proto
