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
    meshes.defer_dtor();
    materials.init(init_capacity, allocator);
    materials.defer_dtor();
    textures.init(init_capacity, allocator);
    textures.defer_dtor();
    cubemaps.init(init_capacity, allocator);
    cubemaps.defer_dtor();
    shader_programs.init(init_capacity, allocator);
    shader_programs.defer_dtor();
}

