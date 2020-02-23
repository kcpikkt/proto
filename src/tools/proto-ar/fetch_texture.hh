#pragma once

#include "proto/core/graphics.hh" 
#include "proto/core/util/namespace-shorthands.hh" 
#include "proto/core/util/StringView.hh" 
#include "proto/core/debug/logging.hh" 


proto::AssetHandle fetch_texture(proto::StringView filepath);
