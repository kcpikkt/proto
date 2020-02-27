#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/math/common.hh"
#include "proto/core/util/Bitset.hh"
#include "proto/core/entity-system/components.hh"
#include "proto/core/entity-system/entity.hh"
#include "proto/core/graphics/Framebuffer.hh"

namespace proto {

struct Component;
struct InvalidComp : Component{};

struct TransformComp;
struct RenderMeshComp;
struct PointlightComp;

using CompTList = meta::typelist<InvalidComp, TransformComp, RenderMeshComp, PointlightComp>;

// Below is the main components typelist, based on it components arrays are created
    
using CompTypeIndex = u8;

template<u8 _index, const char * _name> struct _CompType {
    constexpr static CompTypeIndex index = _index;
    constexpr static const char * name = _name;
    //    constexpr static u32 hash = hash::crc32(_name);
};

struct RuntimeCompType {};
template<typename = RuntimeCompType> struct CompType;

#define PROTO_COMPONENT_TYPE(TYPE)                     \
    template<> struct CompType<TYPE> {                   \
        using type = TYPE;                                    \
        constexpr static CompTypeIndex index = CompTList::index_of<TYPE>::value; \
        constexpr static const char * name = PROTO_STR(TYPE); \
    };
 

PROTO_COMPONENT_TYPE(InvalidComp);
PROTO_COMPONENT_TYPE(TransformComp);
PROTO_COMPONENT_TYPE(RenderMeshComp);
PROTO_COMPONENT_TYPE(PointlightComp);

template<typename T = RuntimeCompType> using CompT = CompType<T>;

// Runtype type information lookup table for O(1) comptypeinfo
struct _CompTypeData {
    template<size_t I>
    using CompAt = typename CompTList::template at_t<I>;

    struct Data {
        const char * name;
        CompTypeIndex index;
        u64 size;
    };

    template<typename...> struct _Lookup;
    template<size_t...Is> struct _Lookup<meta::sequence<Is...>> {
        template<size_t I> constexpr static Data make_data() {
            return {CompType<CompAt<I>>::name, CompType<CompAt<I>>::index, sizeof(CompAt<I>)};
        }

        static_assert(sizeof...(Is) == CompTList::size);
        inline static Data table[sizeof...(Is)] = { make_data<Is>()...};
    };
    using Lookup = _Lookup<meta::make_sequence<0,CompTList::size>>;

    static constexpr Data& at(u64 i) {
        return Lookup::table[i];
    }
};

template<typename T>
struct CompType {
    const CompTypeIndex _index;
    
    constexpr CompType(CompTypeIndex index)
        : _index(index)
    {}

    constexpr auto name() {
        return _CompTypeData::at(_index).name;}

    constexpr auto index() {
        return _CompTypeData::at(_index).index;}

    constexpr auto size() {
        return _CompTypeData::at(_index).size;}
};
    //using Component_List = meta::typelist<TransformComp,
    //                                      RenderMeshComp>
using CompBitset = Bitset<CompTList::size>;

struct EntityMetadata {
    Entity entity;
    CompBitset comps;

    template<typename T>
    bool has_comp() {
        static_assert(meta::is_base_of_v<Component, T>);
        return comps.at((u64)CompTList::index_of<T>::value);
    }

    template<typename T>
    void set_comp() {
        static_assert(meta::is_base_of_v<Component, T>);
        comps.set((u64)CompTList::index_of<T>::value);
    }

    u64 sum_comps_size();
};


} // namespace proto

