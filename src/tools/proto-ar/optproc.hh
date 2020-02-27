#pragma once
#include "proto/core/containers/Array.hh"
#include "proto/core/util/StringView.hh"

proto::Err opt_input_proc(proto::Array<proto::StringView>&);
proto::Err opt_output_proc(proto::Array<proto::StringView>&);
proto::Err opt_search_proc(proto::Array<proto::StringView>&);
proto::Err opt_verbose_proc(proto::Array<proto::StringView>&);
proto::Err opt_preview_proc(proto::Array<proto::StringView>&);
proto::Err opt_list_proc(proto::Array<proto::StringView>&);

