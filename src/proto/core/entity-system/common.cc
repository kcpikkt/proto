#include "proto/core/entity-system/common.hh"

namespace proto {

    //struct _CompTypeData {
    //    template<size_t I>
    //    using CompAt = typename CompTList::template at_t<I>;
    //
    //    struct Data {
    //        const char * name;
    //        CompTypeIndex index;
    //        u64 size;
    //    };
    //
    //    template<typename...> struct _Lookup;
    //    template<size_t...Is> struct _Lookup<meta::sequence<Is...>> {
    //        template<size_t I> constexpr static Data make_data() {
    //            return {CompType<CompAt<I>>::name, CompType<CompAt<I>>::index, sizeof(CompAt<I>)};
    //        }
    //
    //        static_assert(sizeof...(Is) == CompTList::size);
    //        inline static Data table[sizeof...(Is)] = { make_data<Is>()...};
    //    };
    //    using Lookup = _Lookup<meta::make_sequence<0,CompTList::size>>;
    //
    //    static constexpr Data& at(u64 i) {
    //        return Lookup::table[i];
    //    }
    //};


} //namespace proto
