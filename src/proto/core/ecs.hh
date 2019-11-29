#pragma once

#include <cassert>
#include <cstdint>
#include <vector>
#include <variant>
#include <algorithm>
#include <functional>
#include <unordered_map>

// template<typename Component_Type>
// struct System{};

// struct FooComponent {};

// // struct Foo : public System<FooComponent>;

// struct Foo : public System<FooComponent> {
//     inline static int ok = 11;
// };

// struct Bar : public System<FooComponent> {
//     // inline static int ok = 8;
// };

// using SystemsList_Foo = typename SYSTEMS_LIST::append<Foo>::type;
// #undef SYSTEMS_LIST
// #define SYSTEMS_LIST SystemsList_Foo

// using SystemsList_Bar = typename SYSTEMS_LIST::append<Bar>::type;
// #undef SYSTEMS_LIST
// #define SYSTEMS_LIST SystemsList_Bar

// template <typename...> struct Test;

// using create_component_fptr_t = (*)();

// template <template <typename...> typename List,
//           typename ...Ts>
// struct Test <List<typelist_void, Ts...>>{
//     inline static int size = sizeof...(Ts);
//     inline static int size2 = List<typelist_void, Ts...>::size;
//     inline static int arr [List<typelist_void, Ts...>::size] =
//         {Ts::ok...};
namespace ecs {

    using id_t = uint64_t;
    namespace {
        id_t id_counter = 1;
        id_t next_id() {
            return id_counter++;
        }
    };


    struct Entity {
        id_t id = 0;

        bool operator==(const Entity& other) {
            return other.id == id;
        }

        Entity() {}
        Entity(const Entity& other) : id(other.id) {}
    private:
        Entity(id_t _id) : id(_id) {}

        friend Entity add_entity();
    };

    std::vector<Entity> entities;

    struct _System;
    std::vector<_System * > systems;

    Entity add_entity() {
        entities.push_back(Entity(next_id()));
        return entities.back();
    }

    void remove_entity(Entity entity) {
        auto it = std::find(entities.begin(),
                            entities.end(),
                            entity);

        if(it != entities.end()) entities.erase(it);
    }


    struct Component {};

    struct _System {
        using component_t = void;

        _System();
        ~_System();

        virtual void update(float) = 0;
    };

    // entit.add_component<RectTextareaComponent>
    template<typename ComponentType>
    struct System : public _System {
        using component_t = ComponentType;

        static_assert(std::is_base_of<Component, ComponentType>::value);
        static_assert(std::is_default_constructible<ComponentType>::value);

        std::function< size_t(const Entity&) > entity_hash =
            [](const ecs::Entity& entity) -> size_t{
                return std::hash<size_t>{}(entity.id);
            };

        std::function< size_t(const Entity&, const Entity&) > entity_equal =
            [](const Entity& lhs, const Entity& rhs) -> size_t{
                return lhs.id == rhs.id;
            };

        using components_map_t = std::unordered_map<Entity,
                                                    ComponentType,
                                                    decltype(entity_hash),
                                                    decltype(entity_equal)>;

        components_map_t components = components_map_t(10, entity_hash, entity_equal);

        void add_component_to(Entity entity,
                              ComponentType component = ComponentType());

        ComponentType& get_component_mut(Entity entity);

        const ComponentType& get_component(Entity entity);
    };
}
