#pragma once 
#include "proto/core/util/StringView.hh"
#include "proto/core/util/namespace-shorthands.hh"
#include "proto/core/platform/api.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/asset-system/common.hh"
#include "ar-common.hh"

#include "proto/core/asset-system/common.hh" 
#include "proto/core/util/StringView.hh" 

proto::AssetHandle fetch_3d_model(proto::StringView filepath);
proto::AssetHandle fetch_image(proto::StringView filepath);

struct {
    proto::StringView ext;
    proto::AssetHandle (*proc)(proto::StringView);
} ext_handlers[] = {
                 {"obj",  fetch_3d_model},
                 {"png",  fetch_image},
                 {"jpg",  fetch_image},
                 {"jpeg", fetch_image},
                 {"bmp",  fetch_image},
};

proto::AssetHandle fetch(proto::StringView filepath);

