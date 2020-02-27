#include "proto/core/serialization/Archive.hh"
#include "proto/core/math/hash.hh"
#include "proto/core/context.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/entity-system/serialization.hh"
#include "proto/core/debug.hh"
#include "proto/core/entity-system/interface.hh"

namespace proto {

    //Err Archive::_alloc_node(Node * node) {
    //    auto bit = node_bitmap.lsb0();
    //    if(bit == node_bitmap.bitsize)
    //        return nullptr;
    //}

    Err Archive::create(StringView filepath, u64 node_cnt, u64 arena_cap, u64 block_size)
    {
        arena_cap = mem::align(arena_cap, block_size);
        assert(arena_cap % block_size == 0);

        if( !mem::is_pow2(block_size) )
            return INVALID_ARG_ERR;

        println("3");
        auto blocks_cnt = arena_cap / block_size;

        Err ec;
        if( (ec = file.open(filepath, sys::File::read_overwrite_mode)) ) return ec;

        // since node_cnt=max_u32 would be a bug then
        assert( node_cnt != ~((decltype(node_cnt))0) );
        node_cnt += 1; // for root node

        auto superblock_sz_algn   = mem::align( sizeof(Superblock));

        auto arena_bitmap_sz      = blocks_cnt;
        auto arena_bitmap_sz_algn = mem::align( arena_bitmap_sz );

        auto node_bitmap_sz       = node_cnt;
        auto node_bitmap_sz_algn  = mem::align( node_bitmap_sz );

        auto node_table_sz        = node_cnt * sizeof(Node);
        auto node_table_sz_algn   = mem::align( node_table_sz );

        auto header_sz =
            superblock_sz_algn + arena_bitmap_sz_algn + node_bitmap_sz_algn + node_table_sz_algn;

        assert(mem::is_aligned(header_sz));

        auto arena_offset = header_sz;
        auto init_arena_size = _def_init_arena_size;
        auto init_file_size = mem::align(header_sz + init_arena_size);

        if(file.size() < init_file_size)
            if( (ec = file.resize(init_file_size)) ) return ec;
                
        // mapping for supernode & node_table
        // TODO(kacper): add unmapped mode where changes have to be commited manually
        mapping = file.map(sys::File::read_write_mode, {0, header_sz});
        if(!mapping) return IO_FILE_MAP_ERR;

        assert(mapping.size == header_sz);

        memset(mapping.data, 0, header_sz);

        // filling superblock data
        sblk = (Superblock*)mapping.data;

        sblk->signature = ar_superblock_signature;
        strview_copy(sblk->name, filepath);
        sblk->hash = hash::crc32(sblk->name);

        sblk->archive_size = init_file_size;
        sblk->header_sz = header_sz;

        sblk->arena_bitmap_offset  = superblock_sz_algn;
        sblk->arena_bitmap_size    = arena_bitmap_sz;
        sblk->arena_bitmap_bitsize = blocks_cnt;

        sblk->node_bitmap_offset   = sblk->arena_bitmap_size + arena_bitmap_sz_algn;
        sblk->node_bitmap_size     = node_bitmap_sz;
        sblk->node_bitmap_bitsize  = node_cnt;

        sblk->node_table_offset = sblk->node_bitmap_size + node_bitmap_sz_algn;
        sblk->node_table_count  = node_cnt;
        // we are yet to allocate root node
        sblk->node_table_free   = node_cnt;
        sblk->node_table_size   = node_table_sz;

        sblk->block_size = block_size;

        sblk->arena_cap = arena_cap;
        sblk->arena_size = init_arena_size;
        sblk->arena_offset = arena_offset; 

        node_bitmap.place(mapping.data8 + sblk->node_bitmap_offset, sblk->node_bitmap_bitsize);
        arena_bitmap.place(mapping.data8 + sblk->arena_bitmap_offset, sblk->arena_bitmap_bitsize);

        auto nodes_ptr = (Node*)(mapping.data8 + sblk->node_table_offset);
        nodes.init_place_resize(nodes_ptr, sblk->node_table_count);

        u64 root_idx = _alloc_node();
        assert(root_idx == 0);

        nodes[root_idx].type = Node::directory;
        strcpy(nodes[root_idx].name, "nodes[root_idx]");
        nodes[root_idx].hash = hash::crc32(nodes[root_idx].name);

        State::state_init();
        return SUCCESS;
    }
 
    Err Archive::open(StringView filepath, sys::File::Mode filemode){ 
        Err ec;
        if( (ec = file.open(filepath, filemode)) ) return ec;

        Superblock tmp_sblk;
        file.seek(0);
        if( sizeof(Superblock) != file.read( &tmp_sblk, sizeof(Superblock)) ) return ec;

        auto file_sz = file.size();
        if(tmp_sblk.signature != ar_superblock_signature) {
            debug_error(debug::category::data, filepath, " archive signature is corrupted");
            return AR_INVALID_ERR;}

        if(tmp_sblk.archive_size > file_sz) {
            debug_error_fmt(debug::category::data, "Sanity check failed, "
                            "archive claims to be % while its file size smaller - %.",
                            tmp_sblk.archive_size, file_sz);
            return AR_INVALID_ERR;}

        if(tmp_sblk.node_table_size > tmp_sblk.archive_size) {
            debug_error_fmt(debug::category::data, "Sanity check failed, "
                            "archve node_table_offset is % which is greater than "
                            "the size of the file itself - %" ,
                            tmp_sblk.node_table_offset , tmp_sblk.archive_size);
            return AR_INVALID_ERR;}

        auto tmp = tmp_sblk.node_table_offset + tmp_sblk.node_table_size;
        if(tmp > tmp_sblk.archive_size) {
            debug_error(debug::category::data, "Sanity check failed, "
                        "archve node_table_offset + node_table_size is ", tmp,
                        ", which is greater than the size of the archive itself - ",
                        tmp_sblk.archive_size );
            return AR_INVALID_ERR;}

        if(tmp_sblk.arena_offset > tmp_sblk.archive_size) {
            debug_error(debug::category::data, "Sanity check failed, "
                        "archve data_offset ", tmp_sblk.arena_offset,
                        ", which is greater than the size of the archive itself - ",
                        tmp_sblk.archive_size );
            return AR_INVALID_ERR;}

        mapping = file.map(filemode, {0, sblk->header_sz});
        if(!mapping) return IO_FILE_MAP_ERR;

        assert(mapping.size == sblk->header_sz);

        sblk = (Superblock*)mapping.data;

        node_bitmap.place(mapping.data8 + sblk->node_bitmap_offset, sblk->node_bitmap_bitsize);
        arena_bitmap.place(mapping.data8 + sblk->arena_bitmap_offset, sblk->arena_bitmap_bitsize);

        auto nodes_ptr = (Node*)(mapping.data8 + sblk->node_table_offset);
        nodes.init_place_resize(nodes_ptr, sblk->node_table_count);

        State::state_init();
        return SUCCESS;
    }


    Err Archive::_node_sanity_check() {
        auto bitcount0 = node_bitmap.bitcount0();

        if(sblk->node_table_free != bitcount0)
            return DBG_ERR;
        else
            return SUCCESS;
    }

    u64 Archive::_alloc_node() {
        auto free_bit = node_bitmap.lsb<0>();

        if(free_bit != node_bitmap.bitsize) {
            sblk->node_table_free--; 
            node_bitmap.set(free_bit);

            assert(!_node_sanity_check());
            //
            //                println(node_bitmap.bitcount0(), ' ', sblk->node_table_free); 
            //                for(u64 i=0; i<node_bitmap.bitsize; ++i) print((int)node_bitmap.at(i));
            //                println();
            //                flush();
            //                assert(0);
            //            }
            return free_bit;
        } 

        return 0;
    }

    Err Archive::_free_node(u64 idx) {
        if( !node_bitmap.at(idx) )
            return INVALID_ARG_ERR;

        node_bitmap.unset(idx);
        sblk->node_table_free++; 

        assert( !_node_sanity_check() );

        return SUCCESS;
    }

    Err Archive::_arena_alloc(u64 node_idx, u64 bsize) {
        if(!node_idx || node_idx >= nodes.count()) 
            return INVALID_ARG_ERR;

        u64 blk_cnt = mem::align(bsize, sblk->block_size)/sblk->block_size;

        if(arena_bitmap.bitcount<0>() >= blk_cnt) {
            u64 begin = arena_bitmap.find_span<0>( blk_cnt );

            if(begin != arena_bitmap.bitsize) {
                arena_bitmap.set_range(begin, begin + blk_cnt);

                nodes[node_idx].blk_idx = begin;
                nodes[node_idx].blk_cnt = blk_cnt;

                return SUCCESS;
            }
            // TODO(kacper): else defragment?
        }
        return AR_NO_BLOCK_SPAN_ERR;
    }


    Err Archive::store(AssetHandle handle) {
        Err ec;

        if(!handle)
            return INVALID_ARG_ERR;

        MemBuffer cached_mem = get_asset_cached(handle);

        if(!cached_mem)
            return UNIMPL_ERR;

        // 0 is always root node and means error herre
        u64 node_idx = _alloc_node();
        if(!node_idx) return AR_NO_FREE_NODES_ERR;

        // like errdefer from zig
        bool success = false;
        defer { if(!success) _free_node(node_idx); };

        if( (ec = _arena_alloc(node_idx, cached_mem.size)) ) return ec;

        u64 offset = sblk->arena_offset + nodes[node_idx].blk_idx * sblk->block_size;

        if( (ec = file.seek(offset)) )
            return IO_FILE_SEEK_ERR;
            
        if(cached_mem.size != file.write(cached_mem))
            return IO_FILE_WRITE_ERR;

        // this should be now O(1) now as handle should be already hinted
        AssetMetadata* metadata = get_metadata(handle);
        assert(metadata);
        strview_copy(nodes[node_idx].name, metadata->name);

        nodes[node_idx].type = Node::asset;
        nodes[node_idx].parent_index = 0;
        nodes[node_idx].hash = handle.hash;

        success = true;
        return SUCCESS;
    }

    Err Archive::store(Array<Entity>&, ECSTreeMemLayout * const layout) {
        return SUCCESS;
    }
 
    Err Archive::dtor_deep() {
        Err err = SUCCESS;

        file.unmap(mapping);

        if(file.close())
            return IO_FILE_CLOSE_ERR;

        if(nodes.dtor()) // nodes are placed, this is shallow dtor
            return ST_DTOR_ERR;

        return err;
    }

    void Archive::set_name(StringView name) {
        assert(name.length < count_of(sblk->name));

        strview_copy(sblk->name, name);
    }

 
    #if 0
    //namespace serialization {

    //u64 (Array<AssetHandle>& assets) {
    //    u64 acc = sizeof(Archive::Superblock) + (assets.size()+1) * sizeof(Archive::jjjjjjjjjjjjjjjNode);
    //    for(auto asset : assets) {
    //        switch(asset.type) {
    //        case(AssetType<Mesh>::index): acc += ser::serialized_size(get_asset_ref<Mesh>(asset)); break;
    //        case(AssetType<Texture2D>::index): acc += ser::serialized_size(get_asset_ref<Texture2D>(asset)); break;
    //        case(AssetType<Material>::index): acc += ser::serialized_size(get_asset_ref<Material>(asset)); break;
    //        default: assert(0);
    //        }
    //    }
    //    return acc;
    //}

    u8 * Archive::_data_off2addr(u64 offset) {
        //assert(offset < superblock->data_size);
        return (u8*)superblock + superblock->data_offset + offset;
    }

    u64 Archive::_addr2data_off(void * addr) {
        assert(addr >= (u8*)superblock + superblock->data_offset);
        return ((u8*)addr - ((u8*)superblock + superblock->data_offset));
    }

    #if 0 

    // perhaps you can just map file to memory, right?
    Err Archive::_commit_cached_header() {
        auto _cursor = file.cursor();
        file.seek(0);
        if(sizeof(Superblock) != file.write(superblock, sizeof(Superblock)))
            return IO_FILE_WRITE_ERR;
         
        assert(superblock->node_table_size == nodes.size() * sizeof(Node));

        file.seek(superblock->node_table_offset);
        if(superblock->node_table_size != file.write(nodes._data, superblock->node_table_size))
            return IO_FILE_WRITE_ERR;

        file.seek(_cursor);
        return SUCCESS;
    }
    #endif

    Err Archive::create(StringView filepath, u32 node_count, u64 data_size) {
        Err ec;
        if( (ec = file.open(filepath, sys::File::read_overwrite_mode)) ) return ec;

        //FIXME
        node_count += 1; // root //NOTE(kacper): node_count=max_u32 is a bug then

        auto data_offset = sizeof(Superblock) + node_count * sizeof(Node);
        auto size = data_offset + data_size;


        if(file.size() < size)
            if(auto ec = file.resize(size)) return ec;
                
        mapping = file.map(sys::File::read_write_mode, Span{0, size});
        if(!mapping) return IO_FILE_MAP_ERR;

        assert(mapping.size == size);

        superblock = (Superblock*)mapping.data;

        superblock->sig = Superblock::signature;
        strview_copy(superblock->name, filepath);
        superblock->hash = hash::crc32(superblock->name);

        superblock->archive_size = size;
        superblock->node_count = superblock->free_node_count = node_count;

        superblock->node_table_size = node_count * sizeof(Node);
        superblock->node_table_offset = sizeof(Superblock);

        superblock->data_size = 0;
        superblock->data_offset = data_offset; 

        cursor = 0;

        nodes.init_place((u8*)superblock + superblock->node_table_offset, superblock->node_count);
        nodes.zero();
        nodes.resize(nodes.capacity());

        auto& root = nodes[0];
        root.type = Node::directory;
        strcpy(root.name, "root");
        root.hash = hash::crc32(root.name);
        superblock->free_node_count--;

        State::state_init();
        return SUCCESS;
    }
    
    Err Archive::open(StringView filepath /* , sys::File::Mode filemode = sys::File::read_mode */ ) {
        Err ec;
        if( (ec = file.open(filepath, sys::File::read_mode)) ) return ec;

        mapping = file.map(sys::File::read_mode, Span{0, file.size()});
        if(!mapping) return IO_FILE_MAP_ERR;

        superblock = (Superblock*)mapping.data;

        auto file_sz = file.size();
        if(superblock->sig != Superblock::signature) {
            debug_error(debug::category::data, filepath, " archive signature is corrupted");
            return AR_INVALID_ERR;}

        if(superblock->archive_size > file_sz) {
            debug_error_fmt(debug::category::data, "Sanity check failed, "
                            "archive claims to be % while its file size is %.",
                            superblock->archive_size, file_sz);

            return AR_INVALID_ERR;}

        if(superblock->node_table_size > superblock->archive_size) {
            debug_error_fmt(debug::category::data, "Sanity check failed, "
                            "archve node_table_offset is % which is greater than the size of the file itself - %" ,
                            superblock->node_table_offset , superblock->archive_size);

            return AR_INVALID_ERR;}

        auto tmp = superblock->node_table_offset + superblock->node_table_size;
        if(tmp > superblock->archive_size) {
            debug_error(debug::category::data, "Sanity check failed, "
                        "archve node_table_offset + node_table_size is ", tmp,
                        ", which is greater than the size of the file itself - ", superblock->archive_size );
            return AR_INVALID_ERR;}

        // in theory this dont have to hold but whatever
        if(superblock->node_table_offset != sizeof(Superblock)) {
            debug_error(debug::category::data, "Sanity check failed, "
                        "node_table_offset (", superblock->node_table_offset, ") is not equal to sizeof(Superblock) (",
                        sizeof(Superblock), "). (This does not have to hold, delete me after you start maybe 16-align or something)");
            return AR_INVALID_ERR;}

        if(superblock->data_offset > superblock->archive_size) {
            debug_error(debug::category::data, "Sanity check failed, "
                        "archve data_offset ", superblock->data_offset,
                        ", which is greater than the size of the file itself - ", superblock->archive_size );
            return AR_INVALID_ERR;}

        nodes.init_place_resize((u8*)superblock + superblock->node_table_offset, superblock->node_count);

        State::state_init();
        return SUCCESS;
    }

    template<typename T> static MemBuffer _get_asset_cached_mem(T*);

    template<> MemBuffer _get_asset_cached_mem(Mesh * mesh) {
        return mesh && mesh->flags.at(Mesh::cached_bit) ? mesh->cached : MemBuffer{};}

    template<> MemBuffer _get_asset_cached_mem(Texture2D * texture) {
        return texture && texture->flags.check(Texture2D::cached_bit) ? texture->cached : MemBuffer{}; }

    template<> MemBuffer _get_asset_cached_mem(Material * material) {
        return material ? MemBuffer{ {material}, sizeof(Material) } : MemBuffer{}; }


    MemBuffer Archive::get_node_memory(u64 node_idx) {
        return { {_data_off2addr(nodes[node_idx].block_offset)}, nodes[node_idx].block_size };
    }

    // zero if not found, zero is root node
    u64 Archive::_find_free_node() {
        static auto free_node_predecate = [](Node& node){ return node.type == Node::free; };
        u64 idx = nodes.find_if(free_node_predecate);
        assert(idx != 0); // 0 has to be root, can't be free

        return (idx != nodes.size()) ? idx : 0;
    }
    
    Err Archive::store(Array<Entity>& ents, ECSTreeMemLayout * const layout) {
        if(!layout) {
            assert( !calc_ecs_tree_memlayout(*layout, ents) );
        }

        Node node; node.type = Node::ecs_tree;
        node.parent_index = 0;
        node.hash = 0;

        node.block_offset = cursor;
        u8 * new_block = _data_off2addr(cursor);
        u8 * new_block_end = new_block + layout->size;
        u8 * mapping_end = mapping.data8 + mapping.size;

        if( !belongs(new_block, mapping) || !belongs_incl(new_block_end, mapping))
            return MEM_OOM_ERR;

        if(Err ec = serialize_ecs_tree(MemBuffer{{new_block}, layout->size}, ents, *layout))
            return ec;

        //cursor += cached_mem.size;
        //node.block_size = cached_mem.size;
        //node.handle = handle;

        //superblock->data_size += cached_mem.size;

        //memcpy(&nodes[idx], &node, sizeof(Node));

        //superblock->free_node_count--;

        return SUCCESS;
    }
 
    Err Archive::store(AssetHandle handle) {
        u32 idx;

        if( !(idx = _find_free_node()) )
            return AR_NO_FREE_NODES_ERR;

        MemBuffer cached_mem = get_asset_cached(handle);

        if(!cached_mem)
            return UNIMPL_ERR;

        // this should be now O(1) now as handle should be already hinted
        AssetMetadata* metadata = get_metadata(handle);
        assert(metadata);
        strview_copy(node.name, metadata->name);

        u8 * new_block = _data_off2addr(superblock->data_size);
        u8 * new_block_end = new_block + cached_mem.size;
        u8 * mapping_end = mapping.data8 + mapping.size;

        if(  !belongs(new_block, mapping) ||
            (!belongs(new_block_end, mapping) && mapping_end != new_block_end) ) {

            return MEM_OOM_ERR;
        }


        Node node; node.type = Node::asset;
        node.parent_index = 0;
        node.hash = handle.hash;
        memcpy(new_block, cached_mem.data, cached_mem.size);

        node.block_offset = cursor;
        cursor += cached_mem.size;
        node.block_size = cached_mem.size;
        node.handle = handle;

        superblock->data_size += cached_mem.size;

        memcpy(&nodes[idx], &node, sizeof(Node));

        superblock->free_node_count--;

        return SUCCESS;
    }

    AssetHandle Archive::load_asset(u32 index) {
        assert(index < nodes.size());
        auto& node = nodes[index];


        auto h = create_asset(node.name, node.handle.type);

        if(h.type == AssetType<Mesh>::index) {
            //nofail
            auto& mesh = get_asset_ref<Mesh>(h);
            auto& header = *((AssetHeader<Mesh>*)(get_node_memory(index).data));

            mesh.bounds = header.bounds;
            mesh.vertices_count = header.vertices_count;
            mesh.indices_count = header.indices_count;
            if(mesh.indices_count)
                mesh.flags.set(Mesh::indexed_bit);
        }

        if(h != node.handle) return {};

        AssetMetadata * metadata = get_metadata(h); assert(metadata);

        metadata->archive_hash = superblock->hash;
        
        return metadata->handle;
    }
    #endif

    //} // namespace serialization
} // namespace proto
