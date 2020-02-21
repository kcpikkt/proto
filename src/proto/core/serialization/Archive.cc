#include "proto/core/serialization/Archive.hh"
#include "proto/core/math/hash.hh"
#include "proto/core/context.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/entity-system/serialization.hh"
#include "proto/core/debug.hh"
#include "proto/core/entity-system/interface.hh"

namespace proto {
namespace serialization {

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

    void Archive::set_name(StringView name) {
        assert(name.length() >= PROTO_ARCHIVE_MAX_NAME_LEN);

        strview_copy(superblock->name, name);
    }

    #if 0 

    // perhaps you can just map file to memory, right?
    ArchiveErr Archive::_commit_cached_header() {
        auto _cursor = file.cursor();
        file.seek(0);
        if(sizeof(Superblock) != file.write(superblock, sizeof(Superblock)))
            return ArchiveErrCategory::file_write_fail;
         
        assert(superblock->node_table_size == nodes.size() * sizeof(Node));

        file.seek(superblock->node_table_offset);
        if(superblock->node_table_size != file.write(nodes._data, superblock->node_table_size))
            return ArchiveErrCategory::file_write_fail;

        file.seek(_cursor);
        return ArchiveErrCategory::success;
    }
    #endif

    StateErr<Archive> Archive::dtor_deep() {
        StateErr<Archive> err = StateErrCategory<Archive>::success;

        //if(auto ec = _commit_cached_header())
        //    return StateErrCategory<Archive>::dtor_fail;

        file.unmap(mapping);

        if(file.close())
            return StateErrCategory<Archive>::file_close_fail;

        if(nodes.dtor())
            return StateErrCategory<Archive>::dtor_fail;

        // double free just crash
        return err;
    }

    ArchiveErr Archive::create(StringView filepath, u32 node_count, u64 data_size) {
        if(file.open(filepath, sys::File::read_overwrite_mode ))
            return ArchiveErrCategory::file_open_fail;

        //FIXME
        node_count += 1; // root //NOTE(kacper): node_count=max_u32 is a bug then

        auto data_offset = sizeof(Superblock) + node_count * sizeof(Node);
        auto size = data_offset + data_size;


        if(file.size() < size)
            if(file.resize(size))
                return ArchiveErrCategory::file_resize_fail;

        mapping = file.map(sys::File::read_write_mode, Range{0, size});
        if(!mapping) return ArchiveErrCategory::file_mapping_fail;

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
        return ArchiveErrCategory::success;
    }
    
    ArchiveErr Archive::open(StringView filepath /* , sys::File::Mode filemode = sys::File::read_mode */ ) {
        if(file.open(filepath, sys::File::read_mode))
            return ArchiveErrCategory::file_open_fail;

        mapping = file.map(sys::File::read_mode, Range{0, file.size()});
        if(!mapping) return ArchiveErrCategory::file_mapping_fail;

        superblock = (Superblock*)mapping.data;

        auto file_sz = file.size();
        if(superblock->sig != Superblock::signature) {
            debug_error(debug::category::data, filepath, " archive signature is corrupted");
            return ArchiveErrCategory::invalid_archive;}

        if(superblock->archive_size > file_sz) {
            debug_error_fmt(debug::category::data, "Sanity check failed, "
                            "archive claims to be % while its file size is %.",
                            superblock->archive_size, file_sz);

            return ArchiveErrCategory::invalid_archive;}

        if(superblock->node_table_size > superblock->archive_size) {
            debug_error_fmt(debug::category::data, "Sanity check failed, "
                            "archve node_table_offset is % which is greater than the size of the file itself - %" ,
                            superblock->node_table_offset , superblock->archive_size);

            return ArchiveErrCategory::invalid_archive;}

        auto tmp = superblock->node_table_offset + superblock->node_table_size;
        if(tmp > superblock->archive_size) {
            debug_error(debug::category::data, "Sanity check failed, "
                        "archve node_table_offset + node_table_size is ", tmp,
                        ", which is greater than the size of the file itself - ", superblock->archive_size );
            return ArchiveErrCategory::invalid_archive;}

        // in theory this dont have to hold but whatever
        if(superblock->node_table_offset != sizeof(Superblock)) {
            debug_error(debug::category::data, "Sanity check failed, "
                        "node_table_offset (", superblock->node_table_offset, ") is not equal to sizeof(Superblock) (",
                        sizeof(Superblock), "). (This does not have to hold, delete me after you start maybe 16-align or something)");
            return ArchiveErrCategory::invalid_archive;}

        if(superblock->data_offset > superblock->archive_size) {
            debug_error(debug::category::data, "Sanity check failed, "
                        "archve data_offset ", superblock->data_offset,
                        ", which is greater than the size of the file itself - ", superblock->archive_size );
            return ArchiveErrCategory::invalid_archive;}

        nodes.init_place_resize((u8*)superblock + superblock->node_table_offset, superblock->node_count);

        State::state_init();
        return ArchiveErrCategory::success;
    }

    template<typename T> static MemBuffer _get_asset_cached_mem(T*);

    template<> MemBuffer _get_asset_cached_mem(Mesh * mesh) {
        return mesh && mesh->flags.check(Mesh::cached_bit) ? mesh->cached : MemBuffer{};}

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
    
    ArchiveErr Archive::store(Array<Entity>& ents) {

        ECSTreeHeader header;
        u64 comp_cnt[CompTList::size] = {0};

        assert( !create_ecs_tree_header(header, comp_cnt, ents) );

        u8 * new_block = _data_off2addr(superblock->data_size);
        u8 * new_block_end = new_block + header.size;
        u8 * mapping_end = mapping.data8 + mapping.size;

        if( !belongs(new_block, mapping) || !belongs_incl(new_block_end, mapping))
            return ArchiveErrCategory::out_of_memory;

        return serialize_ecs_tree(MemBuffer{{new_block}, header.size}, ents, comp_cnt, header);
    }
 
    ArchiveErr Archive::store(AssetHandle handle) {
        u32 idx;

        if( !(idx = _find_free_node()) )
            return ArchiveErrCategory::no_free_nodes;

        Node node; node.type = Node::asset;
        node.parent_index = 0;
        node.hash = handle.hash;

        MemBuffer cached_mem =
            INVOKE_FTEMPL_WITH_ASSET_PTR(_get_asset_cached_mem, handle);

        if(!cached_mem)
            return ArchiveErrCategory::asset_not_cached;

        // this should be now O(1) now as handle should be already hinted
        AssetMetadata* metadata = get_metadata(handle);
        assert(metadata);
        strview_copy(node.name, metadata->name);

        u8 * new_block = _data_off2addr(superblock->data_size);
        u8 * new_block_end = new_block + cached_mem.size;
        u8 * mapping_end = mapping.data8 + mapping.size;

        if(  !belongs(new_block, mapping) ||
            (!belongs(new_block_end, mapping) && mapping_end != new_block_end) ) {

            return ArchiveErrCategory::out_of_memory;
        }

        memcpy(new_block, cached_mem.data, cached_mem.size);

        node.block_offset = cursor;
        cursor += cached_mem.size;
        node.block_size = cached_mem.size;
        node.handle = handle;

        superblock->data_size += cached_mem.size;

        memcpy(&nodes[idx], &node, sizeof(Node));

        superblock->free_node_count--;

        return ArchiveErrCategory::success;
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

} // namespace serialization
} // namespace proto
