#pragma once
#include "proto/core/common.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/containers/Array.hh"

namespace proto {
    struct AssetContext;

    // TODO(kacper): think maybe about shared pointers?
    struct AssetRegistry {
        Array<AssetMetadata> meshes;
        Array<AssetMetadata> materials;
        Array<AssetMetadata> textures;
        Array<AssetMetadata> cubemaps;
        Array<AssetMetadata> shader_programs;

        memory::Allocator * _allocator;

        void init(size_t init_capacity, memory::Allocator * allocator);
    };
} // namespace proto
