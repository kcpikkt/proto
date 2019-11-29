#pragma once
#include "proto/core/common.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/containers/DynamicArray.hh"

namespace proto {
    struct AssetContext;

    // TODO(kacper): think maybe about shared pointers?
    struct AssetRegistry {
        DynamicArray<AssetMetadata> meshes;
        DynamicArray<AssetMetadata> materials;
        DynamicArray<AssetMetadata> textures;
        memory::Allocator * _allocator;

        void init(size_t init_capacity, memory::Allocator * allocator);
    };
} // namespace proto
