#include "proto/core/entity-system/serialization.hh"
#include "proto/core/entity-system/interface.hh"

namespace proto {

int create_ecs_tree_header
    (ECSTreeHeader& header, u64 (&comp_cnt)[CompTList::size], Array<Entity>& ents)
{
    memset(&header,  0, sizeof header);
    memset(&comp_cnt,  0, sizeof comp_cnt);

    for(auto e : ents) {
        if(auto mdata = get_mdata(e)) {
            // or all comps bitset so we have bitset of all used comps
            header.comp_bits |= mdata->comps;

            for(u64 i=0; i<header.comp_bits.bitsize; ++i)
                if(header.comp_bits.at(i)) comp_cnt[i]++;
        } else
            return -1;
    }

    u64 comp_arr_arena_sz_algn = 0;

    for(u64 i=0; i<header.comp_bits.bitsize; ++i) {
        assert((bool)header.comp_bits.at(i) == (bool)comp_cnt[i]);

        comp_arr_arena_sz_algn +=
            mem::align( comp_cnt[i] * CompType(i).size() );
    }
    assert(mem::is_aligned(comp_arr_arena_sz_algn, 16));

    u64 comp_arrs_count = header.comp_bits.bitcount();

    u64 comp_arrs_arr_sz      = comp_arrs_count * sizeof(ArrayHeader);
    u64 comp_arrs_arr_sz_algn = mem::align(comp_arrs_arr_sz);

    u64 header_sz      = sizeof(ECSTreeHeader);
    u64 header_sz_algn = mem::align(header_sz);

    u64 ent_arr_sz      = sizeof(Pair<Entity, EntityMetadata>) * ents.size();
    u64 ent_arr_sz_algn = mem::align(ent_arr_sz);

    header.size = header_sz_algn + ent_arr_sz_algn + comp_arrs_arr_sz_algn + comp_arr_arena_sz_algn;

    header.ents.offset = header_sz_algn;
    header.ents.size   = ent_arr_sz;
    header.ents.count  = ents.count();

    header.comp_arrs.offset = header.ents.offset + ent_arr_sz_algn;
    header.comp_arrs.size   = comp_arrs_arr_sz;
    header.comp_arrs.count  = comp_arrs_count;

    header.comp_arena_offset = header.comp_arrs.offset + comp_arrs_arr_sz_algn;
    header.comp_arena_size = header.size - header.comp_arena_offset;
    return 0;
}

int serialize_ecs_tree
    (MemBuffer buf, Array<Entity>& ents, u64 (&comp_cnt)[CompTList::size], ECSTreeHeader& header)
{
    // TODO(kacper): THEORETICAL SECURITY ISSUE (if there is need for sec any here)
    //               leaking bytes in align padding, zero buffer or smth

    memcpy(buf.data8, &header, sizeof header);
    // header, done
    assert( ents.count() == header.ents.count );

    for(u64 i=0; i<ents.count(); ++i) {
        // get entity metadata, should never fail
        if(auto mdata = get_mdata(ents[i])) {
            auto arr = (Pair<Entity, EntityMetadata>*)(buf.data8 + header.ents.offset);
            arr[i] = {ents[i], *mdata};
        } else
            return serialization::ArchiveErrCategory::invalid_argument;
    }
    //memcpy(buf.data8 + header.ents.offset, ents.raw(), header.ents.size);
    // ents arr, done
    memset(buf.data8 + header.comp_arrs.offset, 0, header.comp_arrs.size);

    u64 comp_arena_off = header.comp_arena_offset;
    auto comps_bitset = header.comp_bits;

    auto comp_types_cnt = comps_bitset.bitcount();
    for(u32 i=0; i<comp_types_cnt; ++i) {
        u64 bit = comps_bitset.lsb();

        auto comp_arr_header = (ArrayHeader *)
            (buf.data8 + header.comp_arrs.offset + i * sizeof(ArrayHeader));

        comp_arr_header->offset = comp_arena_off;
        comp_arr_header->count = comp_cnt[bit];
        comp_arr_header->size = comp_cnt[bit] * CompType(i).size();

        comp_arena_off += mem::align(comp_arr_header->size);

        comps_bitset.unset(bit);
    }
    // arrays of comp arrs, done

    assert(comps_bitset.is_zero());
    // we filled exacly all cated space
    assert(header.comp_arena_size == (comp_arena_off - header.comp_arena_offset));
    assert(comp_arena_off == header.size);

    // go through all component type we have here
    // take lsb from bitset signifying given component type
    // then go through all entities and check if they have given component
    // if so, fetch it and serialize it to the array
    comps_bitset = header.comp_bits;
    for(u32 i=0; i<comp_types_cnt; ++i) {
        // bit being comp type index
        u64 bit = comps_bitset.lsb();
        // we should not run out of bits before this loop terminates
        assert(bit != comps_bitset.bitsize);
        // how many comps of given type we serialized already
        u64 j = 0;

        // going through entities
        for(auto e : ents) {
            // get entity metadata, should never fail
            if(auto mdata = get_mdata(e)) {
                // if the bit is set, entity has given component
                if(mdata->comps.at(bit)) {
                    // appropriate array header in our array of component arrays
                    auto arrheader = (ArrayHeader *)
                        (buf.data8 + header.comp_arrs.offset + i * sizeof(ArrayHeader));

                    // so this is pointer to where our component should be placed
                    auto ptr = ( buf.data8 + arrheader->offset + j * CompType(bit).size() );

                    // get comp with erased type, no switch case, just metawizardry
                    if(auto comp = get_comp(e, bit)) {
                        memcpy(ptr, comp, CompType(bit).size() );
                    } else {
                        debug_warn(debug::category::main,
                                   CompType(bit).name(), " component bit is set on entity ", e.id,
                                   " but the component isn't there.");
                        return serialization::ArchiveErrCategory::invalid_argument;
                    }

                    // we subtract from array count for given component
                    // checking at the end it should give us only zeros
                    comp_cnt[i]--;
                }

            } else
                return serialization::ArchiveErrCategory::invalid_argument;
        }

        comps_bitset.unset(bit);
        assert(comp_cnt[i] == 0);
    }
    // we should have inspected all bits
    assert(comps_bitset.is_zero());

    return serialization::ArchiveErrCategory::success;
}

int deserialize_ecs_tree (MemBuffer buf)
{
    auto& ctx = *context;
    
    ECSTreeHeader * header = (ECSTreeHeader *)buf.data8;
    if(header->signature != ecs_tree_signature) return -1;
    if(header->size < buf.size) return -2;

    Array<Pair<Entity, EntityMetadata>> ents;
    ents.init_place_resize(buf.data8 + header->ents.offset, header->ents.count);

    // go through ents, fetch all their components, create new one and add them to it
    auto comps = header->comp_bits;
    for(const auto& [ent, mdata] : ents) {
        assert((mdata.comps & header->comp_bits) == mdata.comps);
        auto new_ent = create_entity();

        auto comps = mdata.comps;
        auto bitcount = comps.bitcount();

        for(u64 k=0; k<bitcount; ++k) {
            auto bit = comps.lsb();

            // which one is this in our array?
            auto i = header->comp_bits.bitcount_range(0, bit);
            assert(i < header->comp_arrs.count);

            auto arrheader = (ArrayHeader *)
                (buf.data8 + header->comp_arrs.offset + i * sizeof(ArrayHeader));

            void * from_comp = nullptr;
            for(u64 j=0; j<arrheader->count; ++j) {
                auto comp =
                    (Component*)( buf.data8 + arrheader->offset + j * CompType(bit).size() );

                if(comp->entity == ent) {
                    from_comp = comp;
                    break;
                }
            }
            assert(from_comp);

            if(auto to_comp = add_comp(new_ent, bit)) {
                memcpy(to_comp, from_comp, CompType(bit).size());

                // from_comp had old enitiy id, we swap them here
                ((Component*)to_comp)->entity = new_ent;
            } else {
                debug_error(debug::category::main, "Couldnt add comp to Entity ", new_ent.id );
                return -1;
            }
            comps.unset(bit);
        }
        assert(comps.is_zero());
    }

    return 0;
}

} //namespace proto






     

