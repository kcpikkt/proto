#include "proto/core/asset-system/common.hh"

using namespace proto;

bool AssetHandle::operator==(AssetHandle other) const {
    return (other.hash == hash &&
            other.type == type);
}

bool AssetHandle::is_valid() const {
    return !operator==(AssetHandle{});
}

AssetHandle::operator bool() const{
    return is_valid();
}



