#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/containers/StringArena.hh"
#include "proto/core/util/Buffer.hh"
#include "proto/core/util/Bitfield.hh"
#include "proto/core/containers/ArrayMap.hh"
#include "proto/core/asset-system/common.hh"

extern proto::StringArena search_paths;
extern proto::StringArena loaded_texture_paths;
extern proto::Array<proto::AssetHandle> loaded_assets;
extern proto::Array<proto::MemBuffer> allocated_buffers;

constexpr static proto::u8 cli_preview_bit = BIT(0);
constexpr static proto::u8 cli_verbose_bit = BIT(1);

static proto::Bitfield<proto::u8> cli_flags;
