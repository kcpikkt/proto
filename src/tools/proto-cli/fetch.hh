#pragma once 
#include "proto/core/util/StringView.hh"
#include "proto/core/util/namespace-shorthands.hh"
#include "proto/core/platform/api.hh"
#include "proto/core/debug/logging.hh"
#include "cli-common.hh"

#include "fetch_model_tree.hh"

using namespace proto;


struct {
    StringView ext;
    int (*proc)(StringView);
} ext_handlers[] = {
                 {"obj",  fetch_model_tree},
                 //                 {".png",  fetch_image},
                 //                 {".jpg",  fetch_image},
                 //                 {".jpeg", fetch_image},
                 //                 {".bmp",  fetch_image},
};

int fetch(StringView filepath);
