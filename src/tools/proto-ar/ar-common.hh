#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/containers/StringArena.hh"
#include "proto/core/util/Buffer.hh"
#include "proto/core/util/Bitset.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/entity-system/common.hh"
#include "proto/core/util/argparse.hh"

// all paths passed with -search or -s option
extern proto::StringArena search_paths;
// so we don't load the same textures multiple times
extern proto::StringArena loaded_texture_paths; 
// so we don't archive default assets like cube/plane mesh
extern proto::Array<proto::AssetHandle> loaded_assets;

extern proto::Array<proto::Entity> loaded_ents;
// all buffers allocated for asset storage, can be freed after archivisation
extern proto::Array<proto::MemBuffer> allocated_buffers;

enum {
      verbose_bit,
      preview_bit,
};
extern proto::Bitset<2> ar_flags;

