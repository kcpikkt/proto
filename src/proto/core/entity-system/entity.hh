#pragma once
#include "proto/core/common.hh"

namespace proto {

using EntityId = u32;
using EntityGen = u32;

struct Entity {
    EntityId id = 0;
    EntityGen gen = 0;

    bool operator==(Entity other) const;
    bool is_valid() const;
    operator bool() const;
};
static Entity invalid_entity = Entity{.id = 0, .gen = 0};

}

