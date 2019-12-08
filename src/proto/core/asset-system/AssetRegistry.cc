#include "proto/core/asset-system/AssetRegistry.hh"
#include "proto/core/context.hh"
#include "proto/core/math/hash.hh"

using namespace proto;

void AssetRegistry::init(size_t init_capacity,
          memory::Allocator * allocator)
{
    assert(allocator);
    _allocator = allocator;
    meshes.init(init_capacity, allocator);
    materials.init(init_capacity, allocator);
    textures.init(init_capacity, allocator);
    cubemaps.init(init_capacity, allocator);
}

