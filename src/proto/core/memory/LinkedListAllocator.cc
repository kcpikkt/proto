#include "proto/core/memory/LinkedListAllocator.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/util/bitfield.hh"
#include "proto/core/util/algo.hh"
#include <assert.h>

#if defined(PROTO_DEBUG)
#include "proto/core/io.hh"
#endif

using namespace proto::memory;

void LinkedListAllocator::emplace_init_node()
{
        byte * b_arena = (byte*)_arena;

        //NOTE(kacper): this kind of - block first, then header right behind -
        //              allocation is relevant only when
        //              header alignment < max_alignment
       

        void * init_block = align_forw((void*)(b_arena + sizeof(Header)),
                                        max_alignment);

        Header * init_header = (Header*) get_header(init_block);

        u8 init_block_offset = ((byte*)init_block - (byte*)_arena);

        assert( init_block_offset < _size );

        init_header->size = _size - init_block_offset;
        assert( init_header->size > _min_block_size );

        init_header->magic_number = header_magic_number;
        init_header->first_addr_byte = static_cast<byte>((size_t) init_block);


        init_header->next = nullptr;
        init_header->next_in_mem = nullptr;
        init_header->flags = Header::FREE;

        _first = init_header;
        _first_in_mem = init_header;
        _last = init_header;

        _used += sizeof(Header);

#if defined(PROTO_DEBUG)
#if defined(PROTO_VERBOSE)
        debug_print();
#endif
        sanity_check();
#endif
}

int LinkedListAllocator::init(size_t init_size)
{
    assert(_arena == nullptr);
    assert(_size  == 0);

    assert(init_size > (2 * sizeof(Header) + _min_block_size));

    _arena = (void*) malloc(init_size);

    //TODO(kacper):
    // change that to header size +
    // max_alignment - header alignment + _min_block_size
    _min_alloc_size =
        _min_block_size +
        (2 * sizeof(Header) - 1) +
        (max_alignment - 1);

    if(_arena) {
        assert(is_aligned(_arena, max_alignment));
        _size = init_size;

        emplace_init_node();
        return 0;
    } else {
        return 1;
    }
}

int LinkedListAllocator::init(void * mem, size_t size)
{
    assert(_arena == nullptr);
    assert(_size  == 0);

    assert(size > (2 * sizeof(Header) + _min_block_size));
    assert(mem);

    _arena = mem;

    _min_alloc_size =
        _min_block_size +
        sizeof(Header) +
        (max_alignment - alignof(Header));

    if(_arena) {
        assert(is_aligned(_arena, 16));
        _size = size;

        emplace_init_node();
        return 0;
    } else {
        return 1;
    }
}

int LinkedListAllocator::init(Allocator * other_allocator, size_t size){
    void * data = other_allocator->alloc(size);
    if(!data) {
        //TODO(kacper): print debug markers
        debug_warn(proto::debug::category::memory,
                   "Failed to embed allocator in the other");
        return 1;
    }
    return init(data, size);
}



void * LinkedListAllocator::raw() {
    return _arena;
}

void * LinkedListAllocator::alloc(size_t requested_size) {
    return alloc(requested_size, max_alignment);
}

void * LinkedListAllocator::alloc(size_t requested_size, size_t alignment)
{
    // default max alignment
    alignment = max_alignment;

    assert(is_power_of_two(alignment));
    assert(_arena != nullptr);
    assert(_first != nullptr);
    assert(_size  != 0);

    requested_size = max(requested_size, _min_block_size);

    Header * prev_node = nullptr;
    Header * free_node = find_free_node(requested_size,
                                        &prev_node);

    if(free_node == nullptr) {
        debug_warn(proto::debug::category::memory,
                   "Failed to allocate ", requested_size, " bytes"
                   "(", (requested_size/1024/1024),"mb), allocator out of memory");
        debug_warn(proto::debug::category::memory, "used/size = ",
                   _used , "(", (_used/1024/1024),"mb) /",
                   _size , "(", (_size/1024/1024),"mb)");

        debug_print();
        return nullptr;
    }

    void * ret = get_block(free_node);

    Header * new_node = try_split(free_node, requested_size, prev_node);
    
    if(new_node) {
        insert_sort(free_node, new_node);
    }
    // else if split failed node is still in list, no sort needed

    bitfield_unset(&(free_node->flags), Header::FREE);
    _used += get_header(ret)->size;

#if defined(PROTO_DEBUG)
# if defined(PROTO_VERBOSE)
    debug_print();
# endif
    sanity_check();
#endif
    return ret;
}

void * LinkedListAllocator::realloc(void * block, size_t requested_size) {
    return realloc(block, requested_size, max_alignment);
}

void * LinkedListAllocator::realloc(void * block,
                                    size_t requested_size,
                                    size_t alignment) {
    Header * node = get_header(block);

    assert(node);
    assert(is_aligned(block, max_alignment));
    assert(node->magic_number == header_magic_number);
    assert(node->first_addr_byte == static_cast<byte>((size_t)block));

    if(node->size >= requested_size)
        return block;

    // Due to online merging there should not be any two consecutive
    // free blocks thus we need not to check any but the next one.

    // TODO(kacper): there could be also reallocation expading by
    // previous in memory though it would be useful only in few
    // cases and I dont really think it is worth implementing.
    // But if so, then it should be last check.
    if(node->next_in_mem) {

        size_t expand_size = sizeof(Header) + node->next_in_mem->size;

        if( (node->next_in_mem->flags & Header::FREE) &&
            (node->size + expand_size >= requested_size)) {

            Header * merged_node = merge_with_next_in_mem(node);
            Header * new_node = try_split(merged_node, requested_size);
            if(new_node)
                insert_sort(merged_node, new_node);
            else
                insert_sort(merged_node);

            assert(get_block(merged_node) == block);

#if defined(PROTO_DEBUG)
            sanity_check();
#endif
            return get_block(merged_node);
        }
    }

    // failed to expand, realloc
    void * new_block = alloc(requested_size, alignment);
    if(new_block == nullptr)
        return nullptr;

    size_t cpysize = proto::min(requested_size, node->size);

    memcpy(new_block, get_block(node), cpysize);
    free(block);


#if defined(PROTO_DEBUG)
    sanity_check();
#endif
    return new_block;
}

LinkedListAllocator::Header * 
LinkedListAllocator::try_split(Header * node,
                               size_t needed_size,
                               Header * prev_node){
    // make sure we have aligned spot for header
    void * block = get_block(node);

    byte * tmp_bcursor = (byte*)
        align_forw((byte*) block + needed_size,
                    alignof(Header));

    // shift by header size and get first block aligned address
    void * new_block = align_forw(tmp_bcursor + sizeof(Header),
                                  max_alignment);
    Header * new_node = nullptr;

    byte * b_block_end = (byte*)block + node->size;

    size_t residue = 0;
    if(b_block_end > (byte*)new_block)
        residue = b_block_end - (byte*)new_block;

    if(residue >= _min_block_size){
        // NOTE(kacper): for explanation of this type of allocation
        //               see method emplace_init_node()
        // aka go back to the first header aligned address behind the block
        new_node = get_header(new_block);

        assert(new_node >= (void*)tmp_bcursor);

        size_t padding = (byte*)new_node -
            ((byte*)block + needed_size);

        //printf("%zu %zu\n", requested_size, padding);
        assert((needed_size + padding) % max_alignment == 0);

        assert(tmp_bcursor > (void*)block);

        new_node->size =
            ((byte*)block + node->size) - (byte*)new_block;

        node->size = (byte*)new_node - (byte*)block;

        assert(node->size == needed_size + padding);

        new_node->next_in_mem = node->next_in_mem;
        node->next_in_mem = new_node;

        new_node->magic_number = header_magic_number;
        new_node->first_addr_byte = static_cast<byte>((size_t)new_block);

        new_node->next = nullptr;
        // NOTE(kacper): we dont reset node->next just yet,
        //               it is used later

        bitfield_set(&new_node->flags, Header::FREE);

        _used += sizeof(Header) + get_block_padding(new_node);

        unlink_node(node, prev_node);
        return new_node;
    } else return nullptr; 
}

LinkedListAllocator::Header *
LinkedListAllocator::merge_with_next_in_mem (Header * node) {
    assert(node->magic_number == header_magic_number);
    assert(node->next_in_mem);
    assert(node->next_in_mem->flags & Header::FREE);
    
    size_t expand_size = sizeof(Header) + node->next_in_mem->size;

    Header * new_next_in_mem = node->next_in_mem->next_in_mem;

    //get prev node manually, then reuse
    unlink_node(node->next_in_mem);
    unlink_node(node);

    node->next_in_mem = new_next_in_mem;

    node->size += expand_size;
    _used -= sizeof(Header);

    if( !(node->flags & Header::FREE))
        _used += expand_size;

    //NOTE(kacper): allocator is left now in invalid state the returned 
    //              node is unlinked until it gets insert sorted.
    //              this is because the caller may or may not
    //              want to call try_split before insert_sort

    return node;
}


void LinkedListAllocator::free(void * block) {
    Header * header = get_header(block);

    assert(is_aligned(block, max_alignment));
    assert(header->magic_number == header_magic_number);
    assert(header->first_addr_byte == static_cast<byte>((size_t)block));
    bitfield_set(&header->flags, Header::FREE);
    _used -= header->size;

    if(header->next_in_mem->flags & Header::FREE) {
        insert_sort(merge_with_next_in_mem(header));
    }

#if defined(PROTO_DEBUG)
    sanity_check();
#endif
}

// second, optional argument is to avoid lookup when previous is known
void LinkedListAllocator::unlink_node(Header * node,
                                      Header * prev)
{
    assert(node);
    //if(_first == _last) {
    //    assert(_first == node);
    //    assert(!prev);
    //    _first = nullptr;
    //    _last = nullptr;
    //    node->next = nullptr;
    //} else
    if(prev) {
        assert(prev->next);
        assert(prev->next == node);
        assert(node != _first);

        if(node == _last) {
            assert(node->next == nullptr);
            _last = prev;
        }

        prev->next = node->next;
        node->next = nullptr; 
    } else {
        Header * prev_lookup = nullptr;
        Header * lookup = _first;
        do{
            assert(lookup->magic_number == header_magic_number);

            if(lookup == node){
                if(prev_lookup) {
                    assert(prev_lookup->next);
                    assert(prev_lookup->next == node);
                    assert(node != _first);

                    if(node == _last) {
                        assert(node->next == nullptr);
                        _last = prev_lookup;
                    }

                    prev_lookup->next = node->next;
                    node->next = nullptr; 
                }

                if(lookup == _first) {
                    assert(prev_lookup == nullptr);
                    if(lookup == _last) {
                        assert(lookup->next == nullptr);
                        _first = nullptr;
                        _last = nullptr;
                    } else {
                        _first = lookup->next;
                    }
                }

                lookup->next = nullptr; 
                return;
            }
            prev_lookup = lookup;
            lookup = lookup->next;
        } while(lookup);
        assert(0 && "tried to unlink non existing node" );
    }
}

// NOTE(kacper): there is no reason, except for aesthetic, to not inline this code 
LinkedListAllocator:: Header *
LinkedListAllocator::find_free_node(size_t requested_size, Header ** prev_node){
    assert(_first != nullptr);
    Header * lookup = _first;
    do{
        assert(lookup->magic_number == header_magic_number);

        if( (lookup->flags & Header::FREE) &&
            (lookup->size >= requested_size))
            return lookup;

        if(prev_node != nullptr)
            *prev_node = lookup;
        lookup = lookup->next;
    } while(lookup);

    return nullptr;
}

void LinkedListAllocator::insert_sort(Header * node) {
    assert(node);

    assert(node->next == nullptr);

    if(!_first || !_last) {
        assert(!_first && !_last);
        _first = node;
        _last = node;
        return;
    }

    Header * inserted = node;
    bool node_inserted = false;

    Header * lookup = _first;
    Header * prev_lookup = nullptr;

    while(lookup != nullptr){
        if(inserted->size <= lookup->size) {
            // insert
            inserted->next = lookup;

            if(lookup == _first) {
                assert(prev_lookup == nullptr);
                _first = inserted;
            } else {
                assert(prev_lookup->next == lookup);
                prev_lookup->next = inserted;
            }
            node_inserted = true;
            break;
        }

        assert(lookup != lookup->next);

        prev_lookup = lookup;
        lookup = lookup->next;
    }

    // no hit, push back
    if(!node_inserted) {
        assert(inserted->next == nullptr);
        _last = (_last->next = inserted);
        _last->next = nullptr;
    }

    // why?
    if(_first == nullptr)
        _first = inserted;
}

bool LinkedListAllocator::is_linked(Header * node) {
    if(!node) return false;
    Header * lookup = _first;
    while(lookup){
        if(lookup == node) {
            assert(node != _last || node->next == nullptr);
            return true;
        }
        lookup = lookup->next;
    }
    assert(node->next == nullptr);
    assert(node != _last);
    assert(node != _first);
    return false;
}

void LinkedListAllocator::insert_sort(Header * fst, Header * snd) {
    assert(fst);
    assert(snd);
    assert(fst != snd);

    // nodes have to be unlinked partial check
    assert(fst->next == nullptr);
    assert(snd->next == nullptr);

    assert(!is_linked(fst));
    assert(!is_linked(snd));
                                  
    Header * smaller = ( fst->size <= snd->size ? fst : snd);
    Header * bigger = ( fst->size <= snd->size ? snd : fst);
    assert(smaller != bigger);

    if(!_first || !_last) {
        assert(!_first && !_last);
        _first = smaller;
        _last = bigger;
        _first->next = _last;
        return;
    }

    Header * inserted = smaller;

    bool smaller_inserted = false;
    bool bigger_inserted = false;

    Header * lookup = _first;
    Header * prev_lookup = nullptr;

    //log_info(1, "sort");
    while(lookup != nullptr){
        assert(lookup != inserted);

        if(inserted->size <= lookup->size) {
            // insert
            inserted->next = lookup;

            if(lookup == _first) {
                assert(prev_lookup == nullptr);
                _first = inserted;
            } else {
                assert(prev_lookup->next == lookup);
                prev_lookup->next = inserted;
            }

            if(!smaller_inserted) {
                smaller_inserted = true;
                // we are not starting from the start
                lookup = inserted;
                inserted = bigger;
            } else {
                bigger_inserted = true;
                break;
            }
        }
        assert(lookup != lookup->next);

        prev_lookup = lookup;
        lookup = lookup->next;
    }

    // no hit, push back
    if(!smaller_inserted) {
        assert(smaller->next == nullptr);
        assert(!bigger_inserted);
        _last = (_last->next = smaller);
        _last->next = nullptr;
    }

    if(!bigger_inserted) {
        assert(bigger->next == nullptr);
        _last = (_last->next = bigger);
        _last->next = nullptr;
    }

    if(_first == nullptr)
        _first = smaller;
    
}

#if defined(PROTO_DEBUG)
void LinkedListAllocator::sanity_check() {
    assert(_last->next == nullptr);
    assert(_first != nullptr);
    assert(_first_in_mem != nullptr);
    assert(_last != nullptr);
    assert(_last->next == nullptr);

    size_t size_sum = 0;
    size_t used_sum = 0;

    Header * lookup = _first_in_mem;
    Header * prev_lookup = nullptr;

    size_t nodes_count = 0;
    size_t nodes_in_mem_count = 0;
    while(lookup) {
        size_sum += lookup->size +
            sizeof(Header) +
            get_block_padding(lookup);

        used_sum += sizeof(Header) +
                get_block_padding(lookup);

        if( !(lookup->flags & Header::FREE)) 
            used_sum += lookup->size;

        if(prev_lookup != nullptr) {
            void * expected =
                (byte*)prev_lookup +
                sizeof(Header) +
                get_block_padding(prev_lookup) +
                prev_lookup->size;

            assert_info(expected == (void*)lookup,
                        proto::debug::category::memory,
                        "Address of a linked list node does not match"
                        "with an address of a previous node plus its total size");
        }

        if(lookup->next_in_mem)
            assert(lookup < lookup->next_in_mem);

        prev_lookup = lookup;
        lookup = lookup->next_in_mem;
        nodes_count++;
    }

    //NOTE(kacper): reusing pointers
    lookup = _first;
    prev_lookup = nullptr;
    while(lookup) {
        if(lookup->next) {
            assert(lookup != _last);
            assert(lookup->size <= lookup->next->size);
        }
        prev_lookup = lookup;
        lookup = lookup->next;
        nodes_in_mem_count++;
    }

    assert_info(nodes_count == nodes_in_mem_count,
                proto::debug::category::memory,
                "Size sorted linked list count and memory order linked list count "
                "does not match up ", nodes_count, " != ", nodes_in_mem_count);

    assert_info(size_sum == _size,
                proto::debug::category::memory,
                "LinkedListAllocator sanity check size sum is ", size_sum,
                " while allocator record of its size is ", _size);

    assert_info(used_sum == _used,
                proto::debug::category::memory,
                "LinkedListAllocator sanity check used sum is ", used_sum,
                " while allocator record of used is ", _used);
}

//#if defined(PROTO_VERBOSE)
void LinkedListAllocator::debug_print() {
    Header * lookup = _first_in_mem;
    Header * prev_lookup = nullptr;
    printf("in mem view:\n");
    while(lookup) {
        printf("%p (%zu)", (size_t)((byte*)lookup - (byte*)_arena),
               lookup->size);
        if(lookup->flags & Header::FREE)
            printf(" free");
        printf(" | ");
        prev_lookup = lookup;
        lookup = lookup->next_in_mem;
    }
    printf("\n");

    lookup = _first;
    prev_lookup = nullptr;
    printf("linked list view:\n");
    while(lookup) {
        printf("%zu", lookup->size);
        printf(" -> ");
        prev_lookup = lookup;
        lookup = lookup->next;
    }
    io::print("(null)\n");
    io::print("==============================\n");

    io::flush();

    sanity_check();
}
//#endif

size_t LinkedListAllocator::addr_offset(void * addr) {
    assert(addr >= _arena);
    return (byte*)addr - (byte*)_arena;
}

#endif


//NOTE(kacper): won't work if every block would not be max aligned

LinkedListAllocator::Header *
LinkedListAllocator::get_header(void * addr)
{
    return ((Header*) align_back((void*)((byte*)addr - sizeof(Header)),
                                 alignof(Header)));
}

void * LinkedListAllocator::get_block(Header * header_addr)
{
    return ((void*) align_forw((void*)((byte*)header_addr + sizeof(Header)),
                               max_alignment));
}


proto::u8 LinkedListAllocator::get_block_padding(Header * header_addr) {
    return ((byte*) align_forw((void*)((byte*)header_addr + sizeof(Header)),
                       max_alignment))
        -
        ((byte*)header_addr + sizeof(Header)) ;
}

