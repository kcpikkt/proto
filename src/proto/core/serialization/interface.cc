#include "proto/core/serialization/interface.hh"
#include "proto/core/context.hh"
#include "proto/core/meta.hh"

namespace proto {
namespace serialization {

    Archive * open_archive(StringView filepath) {
        auto& ctx = *context;

        Archive archive;
        if(archive.open(filepath)) return nullptr;

        return &ctx.open_archives.push_back(archive.sblk->hash, meta::move(archive));
    }

} // namespace serialization 
} // namespace proto
