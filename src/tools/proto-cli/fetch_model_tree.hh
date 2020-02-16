#pragma once

#include "cli-common.hh"
#include "fetch_texture.hh"

#include "proto/core/asset-system/interface.hh" 
#include "proto/core/asset-system/serialization.hh" 
#include "proto/core/util/namespace-shorthands.hh" 
#include "proto/core/util/String.hh"

#include "assimp/Importer.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"
#include "assimp/pbrmaterial.h"

#include "proto/core/util/namespace-shorthands.hh" 

using namespace proto;


static vec3 to_vec3(aiColor4D& ai_col) {
    return vec3(ai_col.r, ai_col.g, ai_col.b); }

static vec3 to_vec3(aiVector3D& ai_vec) {
    return vec3(ai_vec.x, ai_vec.y, ai_vec.z); }


static int fetch_model_tree(StringView filepath) {
    // render preview of loaded mesh
    // hmm, but first you need separate thread...
    // or render per load non realtime
    assert(proto::context);
    auto& ctx = *proto::context;

    //texture_paths.init(&ctx.memory);

    assert(filepath.is_cstring());

    if(!platform::is_file(filepath))  {
        log_error(debug::category::main, filepath, " is not a file");
        return -1;
    }

    Assimp::Importer importer;
    const aiScene * scene_ptr =
        importer.ReadFile(filepath.str(),
                          aiProcess_CalcTangentSpace      |
                          aiProcess_GenNormals            |
                          aiProcess_CalcTangentSpace      |
                          aiProcess_Triangulate           |
                          aiProcess_JoinIdenticalVertices |
                          aiProcess_SortByPType);

    if(!scene_ptr) {
        debug_error(debug::category::main, importer.GetErrorString());
        return 1;
    }

    auto& scene = *scene_ptr;

    char asset_name[256];


    // MESHES
    for(u32 m=0; m<scene.mNumMeshes; m++) {
        auto& ai_mesh = (*scene.mMeshes[m]);
        assert(ai_mesh.HasNormals() && "If this fails, you are most likely missing aiProcess_GenNormals."); 
        assert(ai_mesh.HasTangentsAndBitangents() && "If this fails, you are most likely missing aiProcess_CalcTangentSpace."); 

        sprint(asset_name, sizeof(asset_name)/sizeof(char) , sys::basename_view(filepath), '_', m);
        auto& mesh = create_asset_rref<Mesh>(asset_name);

        mesh.vertices_count = ai_mesh.mNumVertices;
        mesh.indices_count = ai_mesh.mNumFaces * 3;

        // some vertices may be reused
        assert(mesh.vertices_count <= mesh.indices_count);

        auto mesh_header = serialization::AssetHeader<Mesh>(mesh);

        MemBuffer buffer = mesh.cached = ctx.memory.alloc_buf(mesh_header.datasize);
        assert(buffer.data);
        allocated_buffers.push_back(buffer);

        mesh.flags.set(Mesh::cached_bit | Mesh::indexed_bit);

        Array<Vertex> vertices;
        vertices.init_place_resize(buffer.data8 + mesh_header.vertices_offset, mesh_header.vertices_count);

        Array<u32> indices;
        indices.init_place_resize(buffer.data8 + mesh_header.indices_offset, mesh_header.indices_count);


        for(u32 v=0; v<ai_mesh.mNumVertices; v++) {
            auto& vert = vertices[v];

            vert.position = to_vec3( ai_mesh.mVertices[v] );
            vert.normal   = to_vec3( ai_mesh.mNormals [v] );
            vert.tangent  = to_vec3( ai_mesh.mTangents[v] );

            if(ai_mesh.HasTextureCoords(0))
                vert.uv = to_vec3( ai_mesh.mTextureCoords[0][v] ).xy();
            else
                vert.uv = {};

            mesh.bounds = {max(mesh.bounds.x, vert.position.x),
                           max(mesh.bounds.y, vert.position.y),
                           max(mesh.bounds.z, vert.position.z)};

        }
        assert(vertices.size() == mesh_header.vertices_count); 
        //tmp
        mesh_header.bounds = mesh.bounds;

        memcpy(buffer.data, &mesh_header, sizeof(mesh_header));

        for(u32 f=0; f<ai_mesh.mNumFaces; f++) {
            auto& ai_face = ai_mesh.mFaces[f];
            assert(ai_face.mNumIndices == 3 && "If this fails, you are most likely missing aiProcess_Triangulate."); 

            indices[f * 3 + 0] = ai_face.mIndices[0];
            indices[f * 3 + 1] = ai_face.mIndices[1];
            indices[f * 3 + 2] = ai_face.mIndices[2];
        }
        assert(indices.size() == mesh_header.indices_count); 

        loaded_assets.push_back(mesh.handle);
    }

    search_paths.store(sys::dirname_view(filepath));

    for(u32 m = 0; m < scene.mNumMaterials; m++) {
        assert(scene.mNumMaterials > m);
        auto& ai_material = *scene.mMaterials[m];
        StringView mat_name = ai_material.GetName().C_Str();
        Material& material = create_asset_rref<Material>(mat_name);

        auto fetch_fail =
            [&](StringView prop_name) {
                debug_warn(debug::category::data,
                            "failed to fetch ", prop_name,
                            " value of material ", mat_name, ".");};

        float tmp_float;
        if(ai_material.Get(AI_MATKEY_SHININESS, tmp_float) == AI_SUCCESS)
            material.trad.shininess = tmp_float;
        else
            fetch_fail("shininess");

        aiColor4D tmp_color;
        if(ai_material.Get(AI_MATKEY_COLOR_DIFFUSE, tmp_color) == AI_SUCCESS)
            material.trad.diffuse = to_vec3(tmp_color);
        else
            fetch_fail("diffuse");

        if(ai_material.Get(AI_MATKEY_COLOR_SPECULAR, tmp_color) == AI_SUCCESS)
            material.trad.specular = to_vec3(tmp_color);
        else
            fetch_fail("specular");

        if(ai_material.Get(AI_MATKEY_COLOR_AMBIENT, tmp_color) == AI_SUCCESS)
            material.trad.ambient = to_vec3(tmp_color);
        else
            fetch_fail("ambient");


        auto texture_find_fail =
            [&](StringView texture_path) {
                debug_warn(debug::category::main,
                           "Could not find texture file ", texture_path, ".");};

        auto texture_fetch_fail =
            [&](StringView prop_name) {
                debug_warn(debug::category::main,
                           "failed to fetch texture ", prop_name,
                           " referenced by material ", mat_name, ".");};

        auto warn_multitex = []() {
                debug_warn(debug::category::main,
                           "multiple textures of the same type "
                           "per mesh are not supported (how?).");};

        aiString ai_texture_path; u32 tex_count;

        tex_count = ai_material.GetTextureCount(aiTextureType_DIFFUSE);
        auto try_fetch_texture =
            [&](aiTextureType ai_texture_type) -> AssetHandle {
                if(tex_count) {
                    if(tex_count > 1) warn_multitex();

                    ai_material.GetTexture(ai_texture_type, 0, &ai_texture_path);

                    String conf_texture_path = sys::search_for_file(ai_texture_path.C_Str(), search_paths);
                    defer { conf_texture_path.dtor(); };

                    if(!conf_texture_path) {
                        texture_find_fail(ai_texture_path.C_Str());
                    } else {
                        if(!loaded_texture_paths.contains(conf_texture_path.view())) {

                            auto h = fetch_texture(conf_texture_path.view());
                            if(h) {
                                loaded_texture_paths.store(conf_texture_path.view());
                                loaded_assets.push_back(h);
                            } else
                                texture_fetch_fail(conf_texture_path.view());

                            return h;
                        }
                    }
                }
                return invalid_asset_handle;
            };

        material.trad.diffuse_map = try_fetch_texture(aiTextureType_DIFFUSE);
        material.trad.specular_map = try_fetch_texture(aiTextureType_SPECULAR);
        material.trad.height_map = try_fetch_texture(aiTextureType_DISPLACEMENT);
        material.trad.normal_map = try_fetch_texture(aiTextureType_NORMALS);

        loaded_assets.push_back(material.handle);
    }



    return 0;
}
