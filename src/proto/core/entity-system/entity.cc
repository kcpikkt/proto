#include "proto/core/entity-system/common.hh"
#include "proto/core/entity-system/components.hh"

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


template<typename...> struct _sum_comps_size_acc;
template<size_t...Is> struct _sum_comps_size_acc<meta::sequence<Is...>> {
    u64 value = 0;

    template<size_t I>
    using Comp = typename CompTList::template at_t<I>;

    _sum_comps_size_acc(const CompBitset& bitset) {
        ((value += sizeof(Comp<Is>) * bitset.at(Is)), ...);
    }
};

u64 EntityMetadata::sum_comps_size() {
    return _sum_comps_size_acc< meta::make_sequence<0, CompTList::size> >(comps).value;
}

} // namespace proto
