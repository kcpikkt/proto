#pragma once
#include "proto/core/entity-system/common.hh"
#include "proto/core/meta.hh"
#include "proto/core/context.hh"

namespace proto {
namespace {
    

    [[nodiscard]]
    Entity create_entity() {
        Entity entity{.id = ++context->entity_generator_data._id, .gen = 0};

        return context->entities.push_back(entity);
    }

    template<typename T> Array<T>& get_comp_arr() {
        static_assert(comp_tlist::contains<T>::value); // or custom ext
        return *((Array<T>*)context->comp_arrs[comp_tlist::index_of<T>::value]);
    }

    template<typename T> T& add_comp(Entity entity) {
        auto& arr = get_comp_arr<T>();

        T comp; comp.entity = entity;
        return arr.push_back(comp);
    }

    template<typename T> T * get_comp(Entity entity) {
        auto& arr = get_comp_arr<T>();

        for(auto& comp : arr)
            if(comp.entity == entity) return &comp;

        return nullptr;
    }

    template<typename TList> struct create_comp_arrays {
        constexpr static auto _arr_sz = sizeof(Array<int>);
        constexpr static auto _init_cap = sizeof(Array<int>);

        template<typename...> struct iterate;
        template<size_t...Is> struct iterate<meta::sequence<Is...>> {

            iterate(MemBuffer buf) {
                // Go thorugh memory in _arr_sz steps, cast to Array<Given Component> *, and init array
                ( ((Array<typename TList::template at<Is>::type> *)
                   (buf.data8 + _arr_sz * Is))->init(_init_cap, &context->memory), ...);

                // Then push approprate poiters to our array
                ( (context->comp_arrs[Is] = buf.data8 + _arr_sz * Is), ...);
            }
        };

        create_comp_arrays() {
            MemBuffer buf = context->memory.alloc_buf(comp_tlist::size * _arr_sz);
            context->comp_arrs.init_resize(comp_tlist::size, &context->memory);

            (void) iterate<meta::make_sequence<0, TList::size>>(buf);
        }
    };


} // namespace (anonymous)
} // namespace proto
