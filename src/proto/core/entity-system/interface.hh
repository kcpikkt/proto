#pragma once
#include "proto/core/entity-system/common.hh"
#include "proto/core/meta.hh"
#include "proto/core/context.hh"

namespace proto {
namespace {
    

    [[nodiscard]]
    Entity create_entity() {
        Entity entity{.id = ++context->entity_generator_data._id, .gen = 0};
        context->ents_mdata.push_back(entity, {});
        return context->ents.push_back(entity);
    }

    template<typename T> Array<T>& get_comp_arr() {
        static_assert(CompTList::contains<T>::value); // or custom ext
        return *((Array<T>*)context->comp_arrs[CompTList::index_of<T>::value]);
    }

    EntityMetadata * get_mdata(Entity entity) {
        proto_assert((bool)entity);

        u64 idx = context->ents_mdata.idx_of(entity);
        if(idx != context->ents_mdata.size())
            return &context->ents_mdata.at_idx(idx);

        return nullptr;
    }

    template<typename T>
    bool has_comp(Entity entity) {
        static_assert(meta::is_base_of_v<Component, T>);

        if(!entity) return false;

        if(auto mdata = get_mdata(entity))
            return mdata->has_comp<T>();

        return false;
    }

    template<typename T>
    T * add_comp(Entity entity) {

        if(!entity){
            debug_error(debug::category::main, "Attempt to add component to invalid entity.");
            return nullptr;
        }

        if(auto mdata = get_mdata(entity)) {
            if(!mdata->has_comp<T>()) {
                mdata->set_comp<T>();

                auto& arr = get_comp_arr<T>();

                T comp; comp.entity = entity;
                return &arr.push_back(comp);
            }
            // what to do here? return previous?
            debug_error(debug::category::main, "Double adding component to metadata.");
        }

        debug_error(debug::category::main, "Valid entity has no associated metadata.");
        return nullptr;
    }

    template<typename T> T * get_comp(Entity entity) {
        auto& arr = get_comp_arr<T>();

        for(auto& comp : arr)
            if(comp.entity == entity) return &comp;

        return nullptr;
    }

    // CURSED CODE DISCLAIMER: only for people with strong somach!
    // create_comp_array 
    // Snake case since it is basically a function, name is subject to change, it is called only once anyway.
    // It takes Typelist as a parameter, then
    template<typename TList> struct create_comp_arrays {
        // size of array of any type should be the same
        // TODO(kacper): go and static_asset it somewhere
        constexpr static auto _arr_sz = sizeof(Array<int>);
        
        // initial cap for each of our components array
        // TODO(kacper): we may to be able to easily specify this one for each component
        //               separately with some more template wizardry.
        constexpr static auto _init_cap = 0;

        template<typename...> struct init_arrs;

        // we use this secondary template as it proves to be difficult to
        // expand Is... from meta::sequence without passing it to specialized template
        template<size_t...Is> struct init_arrs<meta::sequence<Is...>> {

            // array of Ith component type in TList
            template<size_t I>
            using CompArr = Array<typename TList::template at<I>::type>;

            init_arrs(MemBuffer buf) {
                // looks very lispy
                // cast each _arr_sz segment of the buffer to array of each component type in TList and init it
                ( ((CompArr<Is> *) (buf.data8 + _arr_sz * Is))->init(_init_cap, &context->memory), ...);

                // Then cast Ith array to void* and push it to our comp_arrs
                ( (context->comp_arrs[Is] = buf.data8 + _arr_sz * Is), ...);
            }
        };

        create_comp_arrays() {
            // NOTE(kacper): This buffer is never freed and does not have to be as currently components
            //               arrays are spawned at the start of the program and persist until its termination.
            //               Alternatively, since this is a struct, we may inherit StateCRTP and add destructor.
            MemBuffer buf = context->memory.alloc_buf(TList::size * _arr_sz);
            context->comp_arrs.init_resize(TList::size, &context->memory);

            // here is where cursed things happen
            (void) init_arrs<meta::make_sequence<0, TList::size>>(buf);
        }
    };


} // namespace (anonymous)
} // namespace proto
