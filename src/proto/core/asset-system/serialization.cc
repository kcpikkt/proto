#include "proto/core/asset-system/serialization.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/context.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/util/algo.hh"
#include "proto/core/util/Buffer.hh"
#include "proto/core/platform/api.hh"
#include "proto/core/math/hash.hh"

namespace proto {
namespace serialization {

    template<typename T>
    void serialize_specific_asset_to_buffer(T*, MemBuffer buffer);

    template<typename T>
    MemBuffer serialize_asset(T* asset, memory::Allocator * allocator) {
        assert(allocator);
        
        AssetMetadata * metadata = get_metadata(asset);
        assert(metadata);
        
        u64 main_header_size = sizeof(AssetFileHeader);
        u64 deps_size = metadata->deps.size() * sizeof(AssetDependency);

        MemBuffer buffer;
        
        // technically it should be next_multiple(next element alignment, size)
        // but it is a lot of writing
        u64 buf_size =
            next_multiple(16, main_header_size) + 
            next_multiple(16, deps_size) + 
            next_multiple(16, asset->serialized_size());
        
        buffer.data8 = (u8*)allocator->alloc(buf_size);
        assert(buffer.data8);
        assert((size_t)buffer.data % 16 == 0);
        
        buffer.size = buf_size;
        
        u8 * main_header_ptr = buffer.data8;
        
        AssetFileHeader main_header;
        
        main_header.handle.type = AssetType<T>::index;
        main_header.handle.hash = asset->handle.hash;
        strcpy((char*)main_header.name, metadata->name);
        main_header.deps_count = metadata->deps.size();
        main_header.deps_size = main_header.deps_count * sizeof(AssetDependency);

        u8 * deps_ptr = (u8*)
            memory::align_forw(main_header_ptr + sizeof(AssetFileHeader),
                               alignof(AssetDependency));
        main_header.deps_offset = deps_ptr - main_header_ptr;
        
        u8 * asset_data_ptr = (u8*)
            memory::align_forw(deps_ptr + deps_size,
                               alignof(AssetHeader<T>));
        main_header.data_offset = asset_data_ptr - main_header_ptr;
        assert(main_header.data_offset >= main_header_size + deps_size);
        
        memcpy(main_header_ptr, &main_header, sizeof(main_header));
        
        for(u64 i =0; i<metadata->deps.size(); i++) {
            AssetHandle dep_handle = metadata->deps[i];
            AssetMetadata * dep_metadata = get_metadata(dep_handle);
            if(!dep_metadata) {
                debug_warn(debug::category::data,
                            "Could not fetch asset dependency metadata "
                            "during asset serialization.");
            }

            AssetDependency serialized_dep;
            serialized_dep.handle = dep_handle;

            strcpy((char*)serialized_dep.name,
                    (dep_metadata) ? dep_metadata->name : "(unknown)");

            assert(make_handle((const char*) serialized_dep.name,
                               serialized_dep.handle.type) ==
                   serialized_dep.handle);

            memcpy(((u8*)deps_ptr + i * sizeof(AssetDependency)),
                    &serialized_dep,
                    sizeof(AssetDependency));
        }

        MemBuffer asset_buffer;
        asset_buffer.data8 = asset_data_ptr;
        asset_buffer.size  = (buffer.data8 + buffer.size) - asset_data_ptr;

        serialize_specific_asset_to_buffer (asset, asset_buffer); 

        return buffer;
    }

    template<>
    void serialize_specific_asset_to_buffer<Mesh>(Mesh* mesh, MemBuffer buffer)
    {
        assert(buffer.size >= mesh->serialized_size());

        u8 * mesh_header_ptr = buffer.data8;

        AssetHeader<Mesh> mesh_header = mesh->serialization_header_map();

        u8 * vertices_ptr = mesh_header_ptr + mesh_header.vertices_offset;
        u8 * indices_ptr = mesh_header_ptr + mesh_header.indices_offset;
        u8 * spans_ptr = mesh_header_ptr + mesh_header.spans_offset;

        memcpy(mesh_header_ptr, &mesh_header, sizeof(AssetHeader<Mesh>));
        memcpy(vertices_ptr, mesh->vertices.raw(), mesh_header.vertices_size);
        memcpy(indices_ptr,  mesh->indices.raw(),  mesh_header.indices_size);
        memcpy(spans_ptr,    mesh->spans.raw(),    mesh_header.spans_size);
    }
 
    template<>
    void serialize_specific_asset_to_buffer<Texture>(Texture* texture,
                                                     MemBuffer buffer)
    {
        assert(buffer.size >= texture->serialized_size());
        assert(texture->data);

        u8 * texture_header_ptr = buffer.data8;

        AssetHeader<Texture> texture_header = texture->serialization_header_map();

        u8 * texture_data_ptr = texture_header_ptr + texture_header.data_offset;

        memcpy(texture_header_ptr, &texture_header, sizeof(AssetHeader<Texture>));
        memcpy(texture_data_ptr, texture->data, texture_header.data_size);
    }
 
    template<typename T>
    void deserialize_specific_asset_buffer(T * asset, MemBuffer buffer);

    template<>
    void deserialize_specific_asset_buffer<Mesh>(Mesh * mesh, MemBuffer buffer) {
        AssetHeader<Mesh> mesh_header;
        u64 mesh_header_size = sizeof(AssetFileHeader);
        memcpy(&mesh_header, buffer.data, mesh_header_size);

        u8 * vertices_ptr = buffer.data8 + mesh_header.vertices_offset;
        u8 * indices_ptr = buffer.data8 + mesh_header.indices_offset;
        u8 * spans_ptr = buffer.data8 + mesh_header.spans_offset;

        mesh->vertices.resize(mesh_header.vertices_count);
        mesh->indices.resize(mesh_header.indices_count);
        mesh->spans.resize(mesh_header.spans_count);

        assert(mesh->serialized_vertices_size() == mesh_header.vertices_size);
        assert(mesh->serialized_indices_size()  == mesh_header.indices_size);
        assert(mesh->serialized_spans_size()    == mesh_header.spans_size);

        memcpy(mesh->vertices.raw(), vertices_ptr, mesh_header.vertices_size);
        memcpy(mesh->indices.raw(), indices_ptr, mesh_header.indices_size);
        memcpy(mesh->spans.raw(), spans_ptr, mesh_header.spans_size);
    }

    void deserialize_specific_asset_buffer(AssetHandle handle, MemBuffer buffer) {
        switch(handle.type){
        case AssetType<Mesh>::index: {
            Mesh * asset = get_asset<Mesh>(handle);
            assert(asset);
            deserialize_specific_asset_buffer(asset, buffer);
        } break;
            //case AssetType<Material>::index: {
            //    Material * asset = get_asset<Material>(handle);
            //    assert(asset);
            //    deserialize_specific_asset_buffer(asset, buffer);
            //} break;
        case AssetType<Texture>::index: {
            Texture * asset = get_asset<Texture>(handle);
            assert(asset);
            deserialize_specific_asset_buffer(asset, buffer);
        } break;
        default: {
            assert(0);
        }
        }
    }

    AssetHandle deserialize_asset_buffer(MemBuffer buffer) {
        AssetFileHeader main_header;
        u64 main_header_size = sizeof(AssetFileHeader);
        memcpy(&main_header, buffer.data, main_header_size);
        assert(main_header.signature == asset_file_signature);

        AssetHandle handle =
            create_asset((const char*)main_header.name, "",
                         main_header.handle.type);
        assert(handle = main_header.handle);

        AssetMetadata *  metadata = get_metadata(handle);
        assert(metadata);

        assert(main_header.deps_size ==
               main_header.deps_count * sizeof(AssetDependency));

        u8* deps_ptr = buffer.data8 + main_header.deps_offset;
        metadata->deps.resize(main_header.deps_count);

        for(u64 i=0; i<main_header.deps_count; i++){
            AssetDependency * serialized_dep = (AssetDependency*)
                ((u8*) deps_ptr + i * sizeof(AssetDependency));

            assert(make_handle((const char*) serialized_dep->name,
                               serialized_dep->handle.type) ==
                   serialized_dep->handle);

            metadata->deps[i] = serialized_dep->handle;
        }
        u8 * asset_header_ptr = buffer.data8 + main_header.data_offset;
        u64 asset_data_size = (buffer.data8 + buffer.size) - asset_header_ptr;
        MemBuffer asset_buffer = {.data8 = asset_header_ptr,
                                  .size  = asset_data_size };

        deserialize_specific_asset_buffer(handle ,asset_buffer);

        return handle;
    }

    template<typename T>
    int save_asset(T * asset,
                   const char * path,
                   [[maybe_unused]]AssetContext * asset_context)
    {
        namespace sys = proto::platform;

        //TODO(kacper): use asset context allocator
        memory::Allocator * allocator = &proto::context->memory;
        MemBuffer buffer = serialize_asset(asset, allocator);

        sys::File file;
        assert(!file.open(path, sys::file_write));
        assert(!file.size());

        assert( buffer.size == file.write(buffer.data, buffer.size) );

        file.close();

        return 0;
    }
    template int save_asset<Mesh>(Mesh*, const char*, AssetContext*);
    template int save_asset<Texture>(Texture*, const char*, AssetContext*);

    int save_asset(AssetHandle handle,
                   const char * path,
                   AssetContext * context )
    {
        switch(handle.type){
        case AssetType<Mesh>::index: {
            Mesh * asset = get_asset<Mesh>(handle);
            assert(asset);
            return save_asset(asset, path);
        } break;
            //case AssetType<Material>::index: {
            //    Material * asset = get_asset<Material>(handle);
            //    assert(asset);
            //    return save_asset(asset, path);
            //} break;
        case AssetType<Texture>::index: {
            Texture * asset = get_asset<Texture>(handle);
            assert(asset);
            return save_asset(asset, path);
        } break;
        default: {
            assert(0);
        }
        }
        return -1;
    }

        //template<typename T>
    //int create_asset_tree_savelist_rec(T * asset,
    //                                   StringView path,
    //                                   AssetContext * asset_context)
    //{
    //    assert(asset);
    //    AssetMetadata * metadata = get_metadata(asset->handle);

    //    int ret;
    //    for(u32 i=0; i<metadata->deps.size(); i++) {
    //        //TODO(kacper): do not ignore recursive ret value?
    //        save_asset_tree_rec(metadata->deps[i], path, asset_context);
    //    }

    //    char filename[PROTO_ASSET_MAX_PATH_LEN];
    //    strview_copy(filename, metadata->name);
    //    strview_cat(filename,  AssetType<T>::name);
    //    strview_cat(filename,  ".past");

    //    char outfilepath[PROTO_ASSET_MAX_NAME_LEN];
    //    strview_copy(outfilepath, path);
    //    platform::path_ncat(outfilepath, filename, PROTO_ASSET_MAX_PATH_LEN);

    //    io::println(outfilepath);
    //    return 0;
    //        //save_asset(asset,)
    //}


    int save_asset_tree_rec(AssetHandle handle,
                            StringView dirpath,
                            AssetContext * asset_context)
    {
        assert(asset_context);
        AssetContext & ctx = *asset_context;
        namespace sys = proto::platform;
        // TODO(kacper): access check file exists

        if(!sys::is_directory(dirpath)) {
            debug_error(debug::category::data,
                        "Path ", dirpath, ", passed to ", __func__,
                        " is not a path to directory");
            return -1;
        }
        // we construct such a list first to avoid writing
        // many times to the same asset file.
        // e.g. some materials use the same texture for one thing or
        //      share it between each other.
        Savelist savelist;

        savelist.init(100,&context->memory);
        get_deps_rec(savelist, handle);

        char filepath[PROTO_ASSET_MAX_PATH_LEN];
        for(u32 i=0; i<savelist.size(); i++) {
            AssetMetadata * metadata = get_metadata(savelist[i]);

            strview_copy(filepath, dirpath);
            sys::path_ncat(filepath, metadata->name, PROTO_ASSET_MAX_PATH_LEN);
            strview_cat(filepath, "_");
            strview_cat(filepath, AssetType(savelist[i]).name);
            strview_cat(filepath, ".past");

            if(savelist[i].type != AssetType<Material>::index)
                save_asset(savelist[i], filepath, asset_context);
        }

        return 0;
    }


    AssetHandle load_asset_dir(const char * path,
                           AssetContext * context)
    {
        return invalid_asset_handle;
    }
 

    AssetHandle load_asset(StringView path,
                           AssetContext * context)
    {
        AssetContext & ctx = *context;
        namespace sys = proto::platform;
        assert(!strncmp(sys::extension_substr(path), "past", 4));
        sys::File file;
        assert(!file.open(path, sys::file_read));
        assert( file.size());
        memory::Allocator * allocator = &proto::context->memory;

        void * filebuf = allocator->alloc(file.size());
        assert(filebuf);
        assert( file.size() == file.read(filebuf, file.size()) );

        AssetHandle handle =
            deserialize_asset_buffer(MemBuffer{.data = filebuf,
                                               .size = file.size()});

        AssetMetadata * metadata = get_metadata(handle);
        assert(metadata);
        file.close();
        allocator->free(filebuf);

        return handle;
    }


} // namespace serialization    
} // namespace proto
