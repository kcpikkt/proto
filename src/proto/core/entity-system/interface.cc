#include "proto/core/entity-system/interface.hh"

namespace proto {

    // for doing things, that are usually done compile time, runtime
    // i.e. type erasure at its best 
    struct _Runtime {
        template<size_t I>
        using CompAt = typename CompTList::template at_t<I>;

        // get
        template<typename...> struct _get_switch;
        template<size_t...Is> struct _get_switch<meta::sequence<Is...>> {
            Entity _ent;
            size_t _idx;
            void * _ret = nullptr;

            template<size_t I> inline void _case() {
                if(_ret) return;
                // we compare runtime index with static I,
                if(I == _idx) 
                    if(auto comp = get_comp<CompAt<I>>(_ent)) _ret = (void*)comp;
            }

            // go through all the cases
            _get_switch(Entity ent, size_t idx) : _ent(ent), _idx(idx) {
                ((_case<Is>()),...);
            }
        };

        static void * get_comp_at(Entity ent, size_t idx) {
            if(idx >= CompTList::size) return nullptr;
            return _get_switch< meta::make_sequence<0, CompTList::size> >(ent, idx)._ret;
        }

        // add
        template<typename...> struct _add_switch;
        template<size_t...Is> struct _add_switch<meta::sequence<Is...>> {
            Entity _ent;
            size_t _idx;
            void * _ret = nullptr;

            template<size_t I> inline void _case() {
                if(_ret) return;
                // we compare runtime index with static I,
                if(I == _idx) 
                    if(auto comp = add_comp<CompAt<I>>(_ent)) _ret = (void*)comp;
            }

            // go through all the cases
            _add_switch(Entity ent, size_t idx) : _ent(ent), _idx(idx) {
                ((_case<Is>()),...);
            }
        };

        static void * add_comp_at(Entity ent, size_t idx) {
            if(idx >= CompTList::size) return nullptr;
            return _add_switch< meta::make_sequence<0, CompTList::size> >(ent, idx)._ret;
        }
    };

    void * get_comp(Entity entity, u64 comp_idx) {
        return _Runtime::get_comp_at(entity, comp_idx);
    }

    void * add_comp(Entity entity, u64 comp_idx) {
        return _Runtime::add_comp_at(entity, comp_idx);
    }

} //namespace proto 
