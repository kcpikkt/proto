#include "proto/core/serialization/Archive.hh"
#include "proto/core/math/hash.hh"
#include "proto/core/context.hh"
#include "proto/core/asset-system/interface.hh"

namespace proto {
namespace serialization {

    u64 calc_flat_asset_archive_size(Array<AssetHandle>& assets) {
        u64 acc = sizeof(Archive::Superblock) + (assets.size()+1) * sizeof(Archive::Node);
        for(auto asset : assets) {
            switch(asset.type) {
            case(AssetType<Mesh>::index): acc += ser::serialized_size(get_asset_ref<Mesh>(asset)); break;
            case(AssetType<Texture2D>::index): acc += ser::serialized_size(get_asset_ref<Texture2D>(asset)); break;
            case(AssetType<Material>::index): acc += ser::serialized_size(get_asset_ref<Material>(asset)); break;
            default: assert(0);
            }
        }
        return acc;
    }

    u8 * Archive::_data_offset2addr(u64 offset) {
        assert(offset < superblock->data_size);
        return (u8*)superblock + superblock->data_offset + offset;
    }

    u64 Archive::_addr2data_offset(void * addr) {
        assert(addr >= (u8*)superblock + superblock->data_offset);
        return ((u8*)addr - ((u8*)superblock + superblock->data_offset));
    }

    void Archive::set_name(StringView name) {
        assert(name.length() >= PROTO_ARCHIVE_MAX_NAME_LEN);

        strview_copy(superblock->name, name);
    }

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

        return ArchiveErrCategory::success;
    }

    StateErr<Archive> Archive::destroy_deep() {
        StateErr<Archive> err = StateErrCategory<Archive>::success;

        if(auto ec = _commit_cached_header())
            return StateErrCategory<Archive>::destroy_fail;

        if(file.close())
            return StateErrCategory<Archive>::file_close_fail;

        if(nodes.destroy())
            return StateErrCategory<Archive>::destroy_fail;

        // double free just crash
        context->memory.free(cached_header.data);
        return err;
    }

    ArchiveErr Archive::create(StringView filepath, u32 node_count /* u64 prealloc_data_size = 0 */ ) {
        if(file.open(filepath, sys::File::write_mode)) return ArchiveErrCategory::file_open_fail;

        node_count += 1; // root

        auto header_size = sizeof(Superblock) + node_count * sizeof(Node);

        cached_header = context->memory.alloc_buf(header_size);
        if(!cached_header) return ArchiveErrCategory::header_alloc_fail;

        superblock = (Superblock*)cached_header.data;

        superblock->sig = Superblock::signature;
        strview_copy(superblock->name, filepath);
        superblock->hash = hash::crc32(superblock->name);

        superblock->archive_size = header_size;
        superblock->node_count = superblock->free_node_count = node_count;

        superblock->node_table_size = node_count * sizeof(Node);
        superblock->node_table_offset = sizeof(Superblock);

        superblock->data_size = 0;
        superblock->data_offset = header_size; 

        nodes.init_place((u8*)superblock + superblock->node_table_offset, superblock->node_count);
        nodes.zero();
        nodes.resize(nodes.capacity());

        auto& root = nodes[0];
        root.type = Node::directory;
        strcpy(root.name, "root");
        root.hash = hash::crc32(root.name);
        superblock->free_node_count--;

        if(cached_header.size != file.write(cached_header)) 
            return ArchiveErrCategory::file_write_fail;

        State::state_init();
        return ArchiveErrCategory::success;
    }
    
    ArchiveErr Archive::open(StringView filepath /* , sys::File::Mode filemode = sys::File::read_mode */ ) {
        if(file.open(filepath, sys::File::read_mode)) return ArchiveErrCategory::file_open_fail;

        Superblock test_superblock;
        if(sizeof(Superblock) != file.read(&test_superblock, sizeof(Superblock)))
            return ArchiveErrCategory::file_read_fail;

        auto file_sz = file.size();
        if(test_superblock.sig != Superblock::signature ||
           test_superblock.archive_size > file_sz ||
           test_superblock.node_table_offset > file_sz ||
           test_superblock.node_table_offset + test_superblock.node_table_size > file_sz ||
           //// in theory this dont have to hold but whatever
           test_superblock.node_table_offset != sizeof(Superblock) ||
           test_superblock.data_offset > file_sz
           ) return ArchiveErrCategory::invalid_archive;

        auto header_size = sizeof(Superblock) + test_superblock.node_table_size;

        cached_header = context->memory.alloc_buf(header_size);
        if(!cached_header) return ArchiveErrCategory::header_alloc_fail;

        file.seek(0);
        if(cached_header.size != file.read(cached_header))
            return ArchiveErrCategory::file_read_fail;

        superblock = (Superblock*)cached_header.data;
        nodes.init_place_resize((u8*)superblock + superblock->node_table_offset, superblock->node_count);

        State::state_init();
        return ArchiveErrCategory::success;
    }

    template<typename T> static MemBuffer _get_asset_cached_mem(T*);

    template<> static MemBuffer _get_asset_cached_mem(Mesh * mesh) {
        return mesh && mesh->flags.check(Mesh::cached_bit) ? mesh->cached : MemBuffer{};}

    template<> static MemBuffer _get_asset_cached_mem(Texture2D * texture) {
        return texture && texture->flags.check(Texture2D::cached_bit) ? texture->cached : MemBuffer{}; }

    template<> static MemBuffer _get_asset_cached_mem(Material * material) {
        return material ? MemBuffer{ {material}, sizeof(Material) } : MemBuffer{}; }

    static MemBuffer _get_asset_cached_mem(AssetHandle h) {
        switch(h.type) {
            case AssetType<Mesh>::index:      return _get_asset_cached_mem(get_asset<Mesh>     (h));
            case AssetType<Texture2D>::index: return _get_asset_cached_mem(get_asset<Texture2D>(h));
            case AssetType<Material>::index:  return _get_asset_cached_mem(get_asset<Material> (h));
            default: return {};
        }
    }

    ArchiveErr Archive::store(AssetHandle handle) {
        static auto find_free_node = [](Node& node){ return node.type == Node::free; };

        auto idx = nodes.find(find_free_node);

        assert(idx != 0); // 0 has to be root
        if(idx == nodes.size())
            return ArchiveErrCategory::no_free_nodes;

        Node node; node.type = Node::asset;
        node.parent_index = 0;
        node.hash = handle.hash;

        MemBuffer cached_mem = _get_asset_cached_mem(handle);
        // this should be now O(1) now as handle should be already hinted
        AssetMetadata* metadata = get_metadata(handle);
        assert(metadata);
        strview_copy(node.name, metadata->name);

        if(!cached_mem)
            return ArchiveErrCategory::asset_not_cached;

        node.block_offset = file.cursor();
        node.block_size = cached_mem.size;

        file.seek_end();
        if(cached_mem.size != file.write(cached_mem))
            return ArchiveErrCategory::file_write_fail;

        superblock->data_size += cached_mem.size;

        memcpy(&nodes[idx], &node, sizeof(Node));

        superblock->free_node_count--;

        return ArchiveErrCategory::success;
    }

    AssetHandle load_asset(u32 index) {
        auto h = create_asset<>();
        assert(index < nodes.size());
    }

} // namespace serialization
} // namespace proto
