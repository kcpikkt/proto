#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/math/common.hh"
#include "proto/core/util/Bitset.hh"
#include "proto/core/graphics/Framebuffer.hh"

namespace proto {

struct Component;
struct InvalidComp;
struct TransformComp;
struct RenderMeshComp;
struct PointlightComp;
    
using ComponentTypeIndex = u8;

template<u8 _index, const char * _name> struct _ComponentType {
    constexpr static ComponentTypeIndex index = _index;
    constexpr static const char * name = _name;
    //    constexpr static u32 hash = hash::crc32(_name);
};

struct RuntimeComponentType {};
template<typename = RuntimeComponentType> struct ComponentType;

#define PROTO_COMPONENT_TYPE(TYPE, INDEX)                     \
    template<> struct ComponentType<TYPE> {                   \
        using type = TYPE;                                    \
        constexpr static ComponentTypeIndex index = INDEX;    \
        constexpr static const char * name = PROTO_STR(TYPE); \
    };
 
PROTO_COMPONENT_TYPE(InvalidComp, 0);
PROTO_COMPONENT_TYPE(TransformComp, 1);
PROTO_COMPONENT_TYPE(RenderMeshComp, 2);
PROTO_COMPONENT_TYPE(PointlightComp, 3);

// Below is the main components typelist, based on it components arrays are created
using CompTList = meta::typelist<TransformComp, RenderMeshComp, PointlightComp>;

// default for runtime typeinfo
template<typename T>
struct ComponentType {
    const char * name;
    ComponentTypeIndex index;
    
private:
    template<typename U>
    void map_type_info(){
        name = ComponentType<U>::name;
        index = ComponentType<U>::index;
    }
public:
    ComponentType(ComponentTypeIndex index){
        switch(index){
            //case ComponentType<*>::index:
            //    map_type_info<*>();          break;
        default:
            map_type_info<InvalidComp>();  break;
        }
    }
};
    //using Component_List = meta::typelist<TransformComp,
    //                                      RenderMeshComp>
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

struct EntityMetadata {
    Entity entity;
    Bitset<CompTList::size> comps;

    template<typename T>
    bool has_comp() {
        static_assert(meta::is_base_of_v<Component, T>);
        return comps.at(CompTList::index_of_v<T>);
    }

    template<typename T>
    bool set_comp() {
        static_assert(meta::is_base_of_v<Component, T>);
        return comps.set(CompTList::index_of_v<T>);
    }

};


} // namespace proto

