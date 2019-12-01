#pragma once
#include "proto/core/platform/common.hh"
#include "proto/core/common.hh"
#include "proto/core/util/algo.hh"
#include "proto/core/util/StringView.hh"
#include "proto/core/debug/logging.hh"
#include "proto/core/debug/markers.hh"
#include "proto/core/graphics/Mesh.hh"
#include "proto/core/graphics/Texture.hh"
#include "proto/core/asset-system/interface.hh"
#include "proto/core/context.hh"
#include "proto/core/platform/api.hh"
#include "proto/core/math/hash.hh"
#include "proto/core/string.hh"

//FIXME(kacper): Why stbi does not want to run off my custom allocator?
void * malloc_proxy(size_t sz){
    //debug_info(1, "malloc ", sz);
    return (malloc(sz));
}
#define STBI_MALLOC(SZ)                       \
    (malloc_proxy(SZ))

#define STBI_REALLOC(P,NEWSZ)                 \
    (realloc(P, NEWSZ))

#define STBI_FREE(P)                          \
    (free(P))


//#define STBI_MALLOC(SZ)                       \
//    (proto::context->memory.alloc(SZ))
//
//#define STBI_REALLOC(P,NEWSZ)                 \
//    (proto::context->memory.realloc(P, NEWSZ))
//
//#define STBI_FREE(P)                          \
//    (proto::context->memory.free(P))

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

namespace proto{
    AssetHandle
    parse_asset_file_rec(StringView filepath,
                         AssetContext * asset_context = proto::context,
                         AssetMetadata * dependant_asset = nullptr);

    AssetHandle
    parse_image_asset_file_buffer_rec(u8 * buffer,
                                      u64 size,
                                      StringView filepath,
                                      memory::Allocator * allocator,
                                      AssetContext * asset_context,
                                      AssetMetadata * dependant_asset = nullptr)
    {
        assert(buffer);
        assert(size);
        assert(allocator);
        namespace sys = proto::platform;
        int x,y,n;

        StringView name = sys::basename_view(filepath);
        //debug_info(1,filepath, " => ", name);

        AssetHandle handle = create_asset(name,
                                          sys::dirname_view(filepath),
                                          AssetType<Texture>::index);
        if(!handle) {
            debug_warn(debug::category::data,
                       "Could not create asset for ", name,
                       " in file ", filepath);
            return handle;
        }
        Texture * texture =
            get_asset<Texture>(handle);

        //TODO(kacper): proper message
        if(!texture) debug_warn(1, "texture is null");

        texture->data = stbi_load_from_memory(buffer, size, &x, &y, &n, 0);
        assert(x);
        assert(y);
        assert(n);
        //texture->data = stbi_load(filepath, &x, &y, &n, 0);

        assert(texture->data);
        texture->channels = n;
        texture->size = proto::ivec2(x,y);

        if(dependant_asset)
            add_dependency(dependant_asset, handle);

        return handle;
    }

    AssetHandle
    parse_mtl_asset_file_buffer_rec(u8 * buffer,
                                    u64 size,
                                    StringView filepath,
                                    memory::Allocator * allocator,
                                    AssetContext * asset_context,
                                    AssetMetadata * dependant_asset = nullptr)
    {
        namespace sys = proto::platform;
        // TODO(kacper): precount materials, resize dependant_asset dependencies arr
        //if(dependant_asset)
        //    dependant_asset->deps.reserve();

        auto create_new_material =
            [&](StringView name) -> AssetHandle {
                AssetHandle handle =
                    create_asset(name, "",
                                 AssetType<Material>::index); 

                if(!handle) {
                    debug_warn(debug::category::data,
                               "Could not create asset for ", name,
                               " in file ", filepath);
                    return invalid_asset_handle;
                }

                Material * material =
                    get_asset<Material>(handle);
                //TODO(kacper): proper message
                if(!material) debug_warn(1, "material is null");

                if(dependant_asset)
                    add_dependency(dependant_asset, handle);

                return handle;
            };

        char * buf = (char*)buffer;
        
        assert(buf);

        char * cursor = buf;

        AssetHandle handle = invalid_asset_handle;
        Material * current_mat = nullptr;

        while(cursor < (buf + size)){
            while(is_space( *cursor)) cursor++;

            if(!strncmp(cursor, "newmtl", 6)) {
                cursor += 5;
                while( is_space( *(++cursor) ));

                int namelen = 0;
                while(!is_white( *(cursor + (++namelen)) ));
                assert(namelen < PROTO_ASSET_MAX_NAME_LEN);

                handle = create_new_material(StringView(cursor, namelen));
                current_mat = get_asset<Material>(handle);
                //if(!current_mat) return invalid_asset_handle;
            } else {
                /*  */ if(!strncmp(cursor, "Ka", 2)) {
                    cursor+=2;
                    current_mat->ambient_color = extract_vec3(&cursor);
                } else if(!strncmp(cursor, "Kd", 2)) {
                    cursor+=2;
                    current_mat->diffuse_color = extract_vec3(&cursor);
                } else if(!strncmp(cursor, "Ks", 2)) {
                    cursor+=2;
                    current_mat->specular_color = extract_vec3(&cursor);
                } else if(!strncmp(cursor, "a", 1)) {
                    cursor+=2;
                    current_mat->alpha = extract_float(&cursor);
                } else if(!strncmp(cursor, "Ta", 2)) {
                    cursor+=2;
                    current_mat->alpha = 1.0 - extract_float(&cursor);
                } else if(!strncmp(cursor, "Ns", 2)) {
                    cursor+=2;
                    current_mat->shininess = extract_float(&cursor);
                } else if(!strncmp(cursor, "map_", 4)) {
                    cursor+=4;
                    AssetMetadata * metadata = get_metadata(handle);
                    Texture * texture_already = nullptr;
                    AssetHandle * map = nullptr;
                    StringView texpath;

                    /*  */ if(!strncmp(cursor, "Ka", 2)) {
                        cursor+=2;
                        texpath = extract_name(&cursor);
                        map = &current_mat->ambient_map;

                    } else if(!strncmp(cursor, "Kd", 2)) {
                        cursor+=2;
                        texpath = extract_name(&cursor);
                        map = &current_mat->diffuse_map;

                    } else if(!strncmp(cursor, "Ks", 2)) {
                        cursor+=2;
                        texpath = extract_name(&cursor);
                        map = &current_mat->specular_map;

                    } else if(!strncmp(cursor, "bump", 4)) {
                        cursor+=4;
                        texpath = extract_name(&cursor);
                        map = &current_mat->bump_map;
                    }

                    if(map && texpath) {
                        AssetHandle handle =
                            make_handle(sys::basename_view(texpath),
                                        AssetType<Texture>::index);

                        texture_already = get_asset<Texture>(handle);

                        *map = (texture_already)
                            ? texture_already->handle
                            : parse_asset_file_rec(texpath, asset_context, metadata);
                    }
                }
            }
            next_line(&cursor, buf, size);
        }
        AssetHandle ret = invalid_asset_handle;
        // TODO(kacper): set it to some null state
        if(current_mat) ret = current_mat->handle;
        return ret;
    }

    enum _AssetFileFormatIndex
        {
         obj = 1,
         mtl = 2,
         png = 3,
         jpg = 4
        };
    struct _AssetFileFormat {
        char name[32];
        char extension[12];
    };
    _AssetFileFormat supported_asset_file_formats[] = {
         {"wrong file format", ""},
         {"wavefront object", "obj"},
         {"wavefront material", "mtl"},
         {"portable network graphics", "png"},
         {"", "jpg"},
    };

    const u32 supported_asset_file_formats_count = 
        sizeof(supported_asset_file_formats) /
        sizeof(supported_asset_file_formats[0]);

    u32 verify_file_format(const char * filepath) {
        for(u32 i=0; i<supported_asset_file_formats_count; i++){
            if(platform::strcmp_i(platform::extension_substr(filepath),
                                  supported_asset_file_formats[i].extension)
               == 0) return i;
        }
        return 0;
    }

    // =============================================================
    AssetHandle
    parse_obj_asset_file_buffer_rec(u8 * buffer,
                                    u64 size,
                                    StringView filepath,
                                    memory::Allocator * allocator,
                                    AssetContext * asset_context,
                                    AssetMetadata * dependant_asset = nullptr)
    {
        //TEMP
        bool optimize_for_space = false;

        namespace sys = proto::platform;

        AssetHandle handle = create_asset(sys::basename_view(filepath),
                                          sys::dirname_view(filepath),
                                          AssetType<Mesh>::index);

        if(!handle) {
            debug_warn(debug::category::data,
                       "Could not create asset for parsed file");
            return invalid_asset_handle;
        }

        AssetMetadata * metadata =
            get_metadata(handle);

        Mesh * mesh =
            get_asset<Mesh>(handle);

        char * buf = (char*)buffer;
        assert(buf);

    //   COUNT TAGS
        size_t span_count = 0;
        size_t point_count = 0;
        size_t normal_count = 0;
        size_t uv_count = 0;
        size_t face_count = 0;

        bool has_mainspan = false;  // for all indices before first g tag 

        auto count_line_fields =
            [&](char * ptr) -> int{
                int count = 0;
                for(;;) {
                    while(is_space(*ptr)) ptr++;
                    if(*ptr != '\n' && *ptr != '\0') {
                        count++;
                        while(!is_white(*ptr) && *ptr != '\0') ptr++;
                    } else break;
                }
                return count;
            };

        char * cursor = buf;
        u32 count = 0;
        while(cursor < (buf + size)){
            count++;
            
            if(*cursor == 'v') {
                cursor++;
                     if (*cursor == ' ') point_count++;
                else if (*cursor == 'n') normal_count++;
                else if (*cursor == 't') uv_count++;
                else {
                    //assert_info(0, debug::category::data,
                    //            "unknown specifier in obj file");
                }
            } else if(*cursor == 'f') {
                // f tag appeeared before first g tag, adding main group
                if(span_count == 0)
                    has_mainspan = true;

                cursor++;

                u32 line_fields_count = count_line_fields(cursor);

                assert(line_fields_count == 3 ||
                       line_fields_count == 4);

                face_count += line_fields_count - 2;
            } else if(*cursor == 'g') {
                span_count++;
            } else if(*cursor == 'm') {
                if(strncmp("mtllib", cursor, 6) == 0) {
                    const char * tmp_cursor = cursor + 6;
                    while(is_space(*tmp_cursor)) tmp_cursor++;

                    u32 mtl_filename_len = 0;
                    while(!is_white(*(tmp_cursor + mtl_filename_len)))
                        mtl_filename_len++;

                    //FIXME
                    char mtl_filepath[256];
                    sys::path_ncat (strview_copy(mtl_filepath, sys::dirname_view(filepath)),
                                   tmp_cursor,
                                   mtl_filename_len);

                    parse_asset_file_rec(mtl_filepath,
                                         asset_context,
                                         metadata);
                }
            } else if(*cursor == 'o') {
                //assert(0 && "obj file o specifier is not yet supported");
            }
            
            next_line(&cursor, buf, size);

        }
        if(has_mainspan) span_count++;


        assert(span_count);
// SPAWNING MESH

        vardump("hello");
        mesh->spans.reserve(span_count);
        mesh->spans.zero();
        mesh->spans.resize(span_count);
        //    return metadata.handle;

// FILLING MESH SPANS DATA
        
        cursor = buf;

        size_t span_index = 0;
        bool newspan = true;   // is given face first in its span
        bool firstspan = true; // is span first 
        u32 index_count = 0;
        
        while(cursor < (buf + size)){
            if(*cursor == 'f') {
                cursor++;
                u32 line_fields_count = count_line_fields(cursor);

                assert(line_fields_count == 3 ||
                       line_fields_count == 4);

                if(newspan){
                    mesh->spans[span_index].begin_index = index_count;
                    newspan = false;
                }

                mesh->spans[span_index].index_count +=
                    (line_fields_count - 2) * 3;

                index_count +=
                    (line_fields_count - 2) * 3;
            } else if(*cursor == 'g') {
                assert(span_index == 0 ||
                       mesh->spans[span_index ].begin_index ==
                       mesh->spans[span_index - 1].begin_index +
                       mesh->spans[span_index - 1].index_count);

                if(firstspan && !has_mainspan) {
                    // if given mesh does not have mainspan 
                    // which would normally be at index 0
                    // we use index zero for normal spans
                    firstspan = false;
                } else {
                    span_index++;
                }

                newspan = true;
                assert(span_index < span_count);
            } else if(!strncmp(cursor, "usemtl", 6)) {
                cursor += 6;
                StringView name_view = extract_name(&cursor);
                AssetHandle material_handle = 
                    make_handle(name_view, AssetType<Material>::index);

                Material * material = get_asset<Material>(material_handle);
                if(material) {
                    mesh->spans[span_index].material = *material;
                } else {
                    debug_warn(debug::category::data,
                               "Could not fetch material ",name_view,
                               " while parsing ", filepath);
                }

                //if(!metadata->deps.contains(material_handle)) {
                //    debug_warn(debug::category::data,
                //               "obj file ", filepath, " refers to material ",
                //               name_view, " but it is not present."); 

                //    mesh->spans[span_index].material = invalid_asset_handle;
                //}
            }
            next_line(&cursor, buf, size);
        }

        //    vardump(mesh->spans[i].begin_index);
        //    vardump(mesh->spans[i].index_count);
        //}

        vardump(point_count);
        vardump(normal_count);
        vardump(span_count);
        vardump(uv_count);

        assert(point_count);

// SAVE POSITIONS/NORMALS/INDICES

        Array<vec3> positions;
        set_debug_marker(positions, "parse_obj_asset_file_buffer_rec().positions");
        positions.init_resize(point_count, &context->memory);
        Array<vec3> normals;
        set_debug_marker(positions, "parse_obj_asset_file_buffer_rec().normals");
        if(normal_count) normals.init_resize(normal_count, &context->memory);
        Array<vec2> uvs;
        set_debug_marker(positions, "parse_obj_asset_file_buffer_rec().uvs");
        if(uv_count) uvs.init_resize(uv_count, &context->memory);

        size_t point_index = 0;
        size_t normal_index = 0;
        size_t uv_index = 0;

        cursor = buf;
        while(cursor < (buf + size)) {
            if(*cursor == 'v') {
                cursor++;
                if(*cursor == ' ') {
                    assert(point_index < point_count);

                    positions[point_index] = extract_vec3(&cursor);
                    point_index++;
                } else if (*cursor == 'n') {
                    cursor++;
                    assert(normal_index < normal_count);

                    normals[normal_index] = extract_vec3(&cursor);
                    normal_index++;
                } else if (*cursor == 't') {
                    cursor++;
                    uvs[uv_index].x = extract_float(&cursor);
                    uvs[uv_index].y = extract_float(&cursor);
                    uv_index++;
                }
            }
            next_line(&cursor, buf, size);
        }

// SANITY CHECKS

        assert_info(point_index == point_count,
                    debug::category::data,
                    point_index, " != ", point_count );

        assert_info(normal_index == normal_count,
                    debug::category::data,
                    normal_index, " != ", normal_count );
 
        assert_info(uv_index == uv_count,
                    debug::category::data,
                    uv_index, " != ", uv_count );

        assert_info(index_count == face_count * 3,
                    debug::category::data,
                    index_count, " != ", face_count * 3 );

// ASSEMBLING FINAL VERTICES (OPTIMIZED FOR SPACE)

        struct _VertexIndexSet {
            size_t position_index = 0;
            size_t normal_index = 0;
            size_t uv_index = 0;

            bool operator==(_VertexIndexSet& other) {
                return
                    other.position_index == position_index && 
                    other.normal_index   == normal_index && 
                    other.uv_index       == uv_index;
            }
        };

        Array<_VertexIndexSet> index_sets;
        if(optimize_for_space)
            index_sets.init(point_count * 2, allocator);
        else 
            index_sets.init(index_count, allocator);

        auto extract_index_set =
            [](char ** _ptr) -> _VertexIndexSet{
                char * ptr = *_ptr;
                _VertexIndexSet ret;

                ret.position_index = extract_int(&ptr);

                if(*ptr == '/') {
                    ptr++;
                    if(*ptr != '/')
                        ret.uv_index = extract_int(&ptr);
                    assert(ret.uv_index != 0);
                    if(*ptr == '/') {
                        ptr++;
                        assert(*ptr != '/');
                        if(!is_white(*ptr))
                            ret.normal_index = extract_int(&ptr);
                    }
                }

                *_ptr = ptr;
                return ret;
            };

        u32 test_var = 0;
        auto assign_vertex_set
            = [&](_VertexIndexSet vertex_set) -> u32 {
                  test_var++;
                //if(vertex_set.position_index ==
                //   vertex_set.normal_index   ==
                //   vertex_set.uv_index ) return 0;
                 
                assert(vertex_set.position_index < point_count + 1);
                assert(vertex_set.normal_index < normal_count + 1);
                assert(vertex_set.uv_index < uv_count + 1);

                if(optimize_for_space) {
                    for(u64 i=0; i < index_sets.size(); i++) {
                        if(vertex_set == index_sets[i])
                            return i;
                    }
                }
                index_sets.push_back(vertex_set);
                return (index_sets.size() - 1);
            };

        mesh->indices.reserve(index_count);

        cursor             = buf;
        span_index         = 0;
        firstspan          = true;

        u32 index_index = 0; // index of vertex index
        u32 span_index_count   = 0;
        size_t face_index      = 0;

        while(cursor < (buf + size)) {
            if(*cursor == 'f') {
                cursor++;
                u32 line_fields_count = count_line_fields(cursor);
                assert(line_fields_count == 3 ||
                       line_fields_count == 4);


                u32 vertex_index[4];

                vertex_index[0] = assign_vertex_set(extract_index_set(&cursor));
                vertex_index[1] = assign_vertex_set(extract_index_set(&cursor));
                vertex_index[2] = assign_vertex_set(extract_index_set(&cursor));
                mesh->indices.push_back(vertex_index[0]);
                mesh->indices.push_back(vertex_index[1]);
                mesh->indices.push_back(vertex_index[2]);

                if(line_fields_count == 4) {
                    vertex_index[3] =
                        assign_vertex_set(extract_index_set(&cursor));

                    mesh->indices.push_back(vertex_index[0]);
                    mesh->indices.push_back(vertex_index[2]);
                    mesh->indices.push_back(vertex_index[3]);
                }


                face_index += line_fields_count - 2;
                span_index_count +=
                    (line_fields_count - 2) * 3;
                index_index +=
                    (line_fields_count - 2) * 3;

            } else if(*cursor == 'g') {

                if(firstspan && !has_mainspan) {
                    // if given mesh does not have mainspan 
                    // which would normally be at index 0
                    // we use index zero for normal spans
                    firstspan = false;
                } else {
                    assert(mesh->spans[span_index].index_count ==
                           span_index_count);
                    span_index++;
                }

                assert(span_index < span_count);
                span_index_count = 0;
            }
            next_line(&cursor, buf, size);
        }

// SANITY CHECKS

        for(u32 i = 0; i<mesh->spans.size(); i++)
            assert(mesh->spans[i].index_count != 0);

        assert_info(face_index == face_count,
                    debug::category::data,
                    face_index, " != ", face_count );

        assert_info(index_count == index_index,
                    debug::category::data,
                    index_count, " != ", index_index );

// EMIT VERTICES
        

        float mesh_extreme = 0.0;
        mesh->vertices.resize(index_sets.size());

        s32 lowest_index = 2;
        for(s64 i=0; i < index_sets.size(); i++)
            lowest_index = min(index_sets[i].position_index, lowest_index);

        assert(lowest_index == 1 || lowest_index == 0);
        s32 index_offset = -lowest_index;

        for(s64 i=0; i < index_sets.size(); i++) {
            //assert(index_sets[i].position_index);
            mesh->vertices[i] =
                Vertex{ .position = positions[index_sets[i].position_index
                                              + index_offset],

                        .normal = (index_sets[i].normal_index == 0)
                            ? proto::vec3(0.0)
                            : normals[index_sets[i].normal_index
                                      + index_offset],

                        .uv = (index_sets[i].uv_index == 0)
                            ? proto::vec2(0.0)
                            : uvs[index_sets[i].uv_index
                                  + index_offset]
                };
            mesh_extreme =
                max(glm::length(mesh->vertices[i].position),mesh_extreme);
        }

        // scaling here or in shader?
        float positions_scale = 100.0/mesh_extreme;

        for(s64 i=0; i < mesh->vertices.size(); i++) {
            mesh->vertices[i].position *= positions_scale;
        }

        auto compute_normals =
            [&](Mesh& mesh) {
                assert(mesh.indices.size() % 3 == 0);
                for(size_t i=0; i < mesh.vertices.size(); i++ )
                    mesh.vertices[i].normal = vec3(0.0);

                for(size_t i=0; i < mesh.indices.size(); i += 3 ) {
                    auto& v0 = mesh.vertices[mesh.indices[i + 0]];
                    auto& v1 = mesh.vertices[mesh.indices[i + 1]];
                    auto& v2 = mesh.vertices[mesh.indices[i + 2]];
                    vec3 normal = glm::cross(v1.position - v0.position,
                                             v2.position - v0.position);
                    v0.normal += normal;
                    v1.normal += normal;
                    v2.normal += normal;
                }

                for(size_t i=0; i < mesh.vertices.size(); i++ )
                    mesh.vertices[i].normal =
                        glm::normalize(mesh.vertices[i].normal);
            };

        if(normal_count == 0)
            compute_normals(*mesh);

        return metadata->handle;
    }
    
    //NOTE(kacper): would be cool to have more general kind of
    //              search find file given a set of include dirs
    //search_for_asset_file(StringView filepath)
    //==============================================================
    AssetHandle
    parse_asset_file_rec(StringView filepath,
                         AssetContext * asset_context,
                         AssetMetadata * dependant_asset)
    {

        char _filepath[PROTO_ASSET_MAX_PATH_LEN];
        strview_copy(_filepath, filepath);

        // FIXME(kacper): well idk, I guess we are using backslashes for
        //                every platform anyway
        str_trans(_filepath, [](char c){ return (c == '\\' ? '/' : c); });

        //log_info(1,"parsing asset file ", _filepath);

        AssetHandle ret;

        char finalpath[2 * PROTO_ASSET_MAX_PATH_LEN];

        strview_copy(finalpath, _filepath);

        if(access(finalpath, F_OK) == -1) {

            bool found = false;
            for(s32 i=0; i < asset_context->asset_paths.count(); i++) {
                StringView searchpath = asset_context->asset_paths[i];
                strview_copy(finalpath, searchpath);

                platform::path_ncat(finalpath, _filepath,
                                    2 * PROTO_ASSET_MAX_PATH_LEN);

                if(access(finalpath, F_OK) != -1) {
                    found = true; 
                    break;
                }
            }
            if(!found) {
                debug_warn(debug::category::data,
                        "Could not find asset file ", _filepath);
                io::println("...searched:");

                strview_copy(finalpath, _filepath);

                io::println(finalpath);

                for(s32 i=0; i < asset_context->asset_paths.count(); i++) {
                    StringView searchpath = asset_context->asset_paths[i];
                    strview_copy(finalpath, searchpath);
                    io::println(finalpath);
                    platform::path_ncat(finalpath, _filepath,
                                        2 * PROTO_ASSET_MAX_PATH_LEN);
                    io::println(finalpath);
                }
                io::flush();

                return invalid_asset_handle;
            }
        }

        u32 file_format_index = verify_file_format(finalpath);
        if(!file_format_index) {
            debug_warn(debug::category::data,
                       "Failed to load assets from ", finalpath,
                       ", unsupported external asset file format ");
            return invalid_asset_handle;
        }

        platform::File file;
        assert(!file.open(finalpath, platform::file_read));

        memory::Allocator * allocator = &(context->memory);
        u8 * buf = (u8*)allocator->alloc(file.size());

        assert(buf);
        assert(file.size() == file.read(buf, file.size() ));

        switch(file_format_index) {
        case _AssetFileFormatIndex::obj: {
            ret = parse_obj_asset_file_buffer_rec
                (buf, file.size(), finalpath,
                 &context->memory, asset_context, dependant_asset);
        } break;
        case _AssetFileFormatIndex::mtl: {
            ret = parse_mtl_asset_file_buffer_rec
                (buf, file.size(), finalpath,
                 &context->memory, asset_context, dependant_asset);
        }break;
        case _AssetFileFormatIndex::jpg: // fallthrough 
        case _AssetFileFormatIndex::png: {
            ret = parse_image_asset_file_buffer_rec
                (buf, file.size(), finalpath,
                 &context->memory, asset_context, dependant_asset);
        }break;
        default: {
            assert(0 && "unsupported file");
        }
        }

        file.close();
        allocator->free(buf);

        return ret;
    }


} // namespace proto
