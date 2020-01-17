#pragma once
#include "proto/core/entity-system/common.hh"
#include "proto/core/context.hh"

namespace proto {
namespace {
    
    [[nodiscard]]
    Entity create_entity() {
        Entity entity{.id = ++context->entity_generator_data._id, .gen = 0};

        return context->entities.push_back(entity);
    }

    template<typename T> Array<T>* get_component_array() {
        /**/ if constexpr
            (ComponentType<T>::index == ComponentType<TransformComp>::index)
                return &context->comp.transform;
        else if constexpr
            (ComponentType<T>::index == ComponentType<RenderMeshComp>::index)
                return &context->comp.render_mesh;
        else if constexpr
            (ComponentType<T>::index == ComponentType<PointlightComp>::index)
                return &context->comp.pointlights;
        else {
                return nullptr;
                assert(0);
        }
    }

    template<typename T> T& add_component(Entity entity) {
        Array<T>* arr = get_component_array<T>();
        assert(arr); T comp; comp.entity = entity;
        return arr->push_back(comp);
    }

    template<typename T> T * get_component(Entity entity) {
        Array<T>* lookup_arr = get_component_array<T>();
        assert(lookup_arr);

        for(auto& comp : *lookup_arr)
            if(comp.entity == entity) return &comp;

        return nullptr;
    }


} // namespace (anonymous)
} // namespace proto
