#pragma once
#include "proto/core/containers/Array.hh"
#include "proto/core/util/StringView.hh"

int opt_input_proc(proto::Array<proto::StringView>&);

int opt_output_proc(proto::Array<proto::StringView>&);

int opt_search_proc(proto::Array<proto::StringView>&);

int opt_verbose_proc(proto::Array<proto::StringView>&);

int opt_preview_proc(proto::Array<proto::StringView>&);

int opt_list_proc(proto::Array<proto::StringView>&);

