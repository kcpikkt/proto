#pragma once
#include "proto/core/serialization/Archive.hh"
#include "proto/core/util/Optional.hh"

namespace proto {
namespace serialization {

    Archive * open_archive(StringView filepath);

} // namespace serialization 
} // namespace proto
