#include "proto/core/entity-system/common.hh"

namespace proto {

bool Entity::operator==(Entity other) const {
    return (id == other.id && gen == other.gen);
}

bool Entity::is_valid() const {
    return !(*this == invalid_entity);
};

Entity::operator bool() const {
    return is_valid();
};


} // namespace proto
