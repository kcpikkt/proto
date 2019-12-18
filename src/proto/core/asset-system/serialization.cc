#include "proto/core/asset-system/serialization.hh"
#include "proto/core/asset-system/common.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/context.hh"
#include "proto/core/memory/common.hh"
#include "proto/core/util/algo.hh"
#include "proto/core/util/Buffer.hh"
#include "proto/core/platform/api.hh"
#include "proto/core/math/hash.hh"
#include "proto/core/graphics/Cubemap.hh"

#include "proto/core/graphics/gl.hh"

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
        memcpy(vertices_ptr, (void*)mesh->vertices.raw(), mesh_header.vertices_size);
        memcpy(indices_ptr,  (void*)mesh->indices.raw(),  mesh_header.indices_size);
        memcpy(spans_ptr,    (void*)mesh->spans.raw(),    mesh_header.spans_size);
    }
 
    template<>
    void serialize_specific_asset_to_buffer<Texture2D>(Texture2D* texture,
                                                     MemBuffer buffer)
    {
        assert(buffer.size >= texture->serialized_size());
        assert(texture->data);

        u8 * texture_header_ptr = buffer.data8;

        AssetHeader<Texture2D> texture_header = texture->serialization_header_map();

        u8 * texture_data_ptr = texture_header_ptr + texture_header.data_offset;

        memcpy(texture_header_ptr, &texture_header, sizeof(AssetHeader<Texture2D>));
        memcpy(texture_data_ptr, texture->data, texture_header.data_size);
    }

    template<>
    void serialize_specific_asset_to_buffer<Cubemap>(Cubemap* cubemap,
                                                     MemBuffer buffer)
    {
        assert(buffer.size >= cubemap->serialized_size());

        assert(cubemap->data[0]); assert(cubemap->data[1]);
        assert(cubemap->data[2]); assert(cubemap->data[3]);
        assert(cubemap->data[4]); assert(cubemap->data[5]);

        AssetHeader<Cubemap> cubemap_header = cubemap->serialization_header_map();

        u8 * cubemap_header_ptr = buffer.data8;
        u8 * cubemap_data_ptr = cubemap_header_ptr + cubemap_header.data_offset;

       memcpy(cubemap_header_ptr, &cubemap_header, sizeof(AssetHeader<Cubemap>));

        u64 one_side_size = cubemap_header.data_size / 6;
        u8 * rt_data_ptr = cubemap_data_ptr;
        u8 * lf_data_ptr = rt_data_ptr + one_side_size;
        u8 * up_data_ptr = lf_data_ptr + one_side_size;
        u8 * dn_data_ptr = up_data_ptr + one_side_size;
        u8 * fw_data_ptr = dn_data_ptr + one_side_size;
        u8 * bk_data_ptr = fw_data_ptr + one_side_size;

        memcpy(rt_data_ptr, cubemap->data[0], one_side_size);
        memcpy(lf_data_ptr, cubemap->data[1], one_side_size);
        memcpy(up_data_ptr, cubemap->data[2], one_side_size);
        memcpy(dn_data_ptr, cubemap->data[3], one_side_size);
        memcpy(fw_data_ptr, cubemap->data[4], one_side_size);
        memcpy(bk_data_ptr, cubemap->data[5], one_side_size);
    }
 
 
    template<typename T>
    void deserialize_specific_asset_buffer(T * asset, MemBuffer buffer);

    template<>
    void deserialize_specific_asset_buffer<Texture2D>(Texture2D * texture,
                                                      MemBuffer buffer)
    {
        assert(texture);
        AssetHeader<Texture2D> texture_header;
        u64 texture_header_size = sizeof(AssetHeader<Texture2D>);
        memcpy(&texture_header, buffer.data, texture_header_size);

        u8 * tex_data_ptr = buffer.data8 + texture_header.data_offset;

        memory::Allocator * allocator = &context->memory;
        void * data = allocator->alloc(texture_header.data_size);
        assert(data);
        texture->data = data;
        texture->_allocator = allocator;
        texture->channels = texture_header.channels;
        texture->format = texture_header.format;
        texture->gpu_format = texture_header.gpu_format;
        texture->size = texture_header.size;

        assert(texture_header.data_size == texture->serialized_data_size());

        memcpy(texture->data, tex_data_ptr, texture_header.data_size);
        // NOTE(kacper): here memory is not freed, it should be freed after
        //               gfx::gpu_upload(). Allocator is known since it is pointed
        //               by Texture2D::_allocator but idk if it is good idea.
        //               Perhaps there will be just one texture staging allocator
        //               for that known globally.
        // allocator->free(data);
    }

    template<>
    void deserialize_specific_asset_buffer<Cubemap>(Cubemap * cubemap,
                                                    MemBuffer buffer)
    {
        assert(cubemap);
        AssetHeader<Cubemap> cubemap_header;
        u64 cubemap_header_size = sizeof(AssetHeader<Cubemap>);
        memcpy(&cubemap_header, buffer.data, cubemap_header_size);

        u8 * tex_data_ptr = buffer.data8 + cubemap_header.data_offset;

        memory::Allocator * allocator = &context->memory;
        void * data = allocator->alloc(cubemap_header.data_size);
        assert(data);
        cubemap->channels = cubemap_header.channels;
        cubemap->format = cubemap_header.format;
        cubemap->gpu_format = cubemap_header.gpu_format;
        cubemap->size = cubemap_header.size;

        u64 one_side_size = cubemap_header.data_size / 6;

        // TODO(kacper): reorder when parsing
        cubemap->data[4] = data;
        cubemap->data[5] = (u8*)cubemap->data[4] + one_side_size;
        cubemap->data[2] = (u8*)cubemap->data[5] + one_side_size;
        cubemap->data[3] = (u8*)cubemap->data[2] + one_side_size;
        cubemap->data[0] = (u8*)cubemap->data[3] + one_side_size;
        cubemap->data[1] = (u8*)cubemap->data[0] + one_side_size;

        assert(cubemap_header.data_size == cubemap->serialized_data_size());

        memcpy(data, tex_data_ptr, cubemap_header.data_size);
        // NOTE(kacper): here memory is not freed, it should be freed after
        //               gfx::gpu_upload(). Allocator is known since it is pointed
        //               by Texture2D::_allocator but idk if it is good idea.
        //               Perhaps there will be just one texture staging allocator
        //               for that known globally.
        // allocator->free(data);
    }
 
    template<>
    void deserialize_specific_asset_buffer<Mesh>(Mesh * mesh,
                                                 MemBuffer buffer)
    {
        AssetHeader<Mesh> mesh_header;
        u64 mesh_header_size = sizeof(AssetHeader<Mesh>);
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
        case AssetType<Texture2D>::index: {
            Texture2D * asset = get_asset<Texture2D>(handle);
            assert(asset);
            deserialize_specific_asset_buffer(asset, buffer);
        } break;
        case AssetType<Cubemap>::index: {
            Cubemap * asset = get_asset<Cubemap>(handle);
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
            create_init_asset((const char*)main_header.name, 
                               main_header.handle.type);

        if(handle.type == AssetType<Mesh>::index) {
            assert(get_asset<Mesh>(handle)->State::is_initialized());
        }
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
    template int save_asset<Texture2D>(Texture2D*, const char*, AssetContext*);
    template int save_asset<Cubemap>(Cubemap*, const char*, AssetContext*);

    int save_asset(AssetHandle handle,
                   const char * path,
                   [[maybe_unused]]AssetContext * context )
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
        case AssetType<Texture2D>::index: {
            Texture2D * asset = get_asset<Texture2D>(handle);
            assert(asset);
            return save_asset(asset, path);
        } break;
        case AssetType<Cubemap>::index: {
            Cubemap * asset = get_asset<Cubemap>(handle);
            assert(asset);
            return save_asset(asset, path);
        } break;
        default: {
            assert(0);
        }
        }
        return -1;
    }

    int save_asset_tree_rec(AssetHandle handle,
                            StringView dirpath,
                            AssetContext * asset_context)
    {
        assert(asset_context);
        //AssetContext & ctx = *asset_context;
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


    AssetHandle load_asset_dir(StringView path,
                               AssetContext * asset_context)
    {
        namespace sys = proto::platform;

        log_info(debug::category::data,
                 "Loading assets from directory ", path);
        AssetHandle ret = invalid_asset_handle;
        auto filenames = sys::ls(path);
        for(auto filename : filenames) {
            StringView ext = sys::extension_view(filename);
            char asset_path[256];

            // NOTE(kacper): remember strcmp is stupid
            if(ext.length() == 4 && !strncmp("past", ext, ext.length())) {

                strview_copy(asset_path, path);
                sys::path_ncat(asset_path, filename, 256);
                ret = load_asset(asset_path, asset_context);
            }
        }
        return invalid_asset_handle;
    }
 

    AssetHandle load_asset(StringView path,
                           [[maybe_unused]]AssetContext * context)
    {
        //AssetContext & ctx = *context;
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
