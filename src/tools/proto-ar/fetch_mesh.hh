#pragma once

#include "proto/core/asset-system/interface.hh" 
#include "proto/core/asset-system/serialization.hh" 
#include "proto/core/util/namespace-shorthands.hh" 

#include "assimp/Importer.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "proto/core/util/namespace-shorthands.hh" 

using namespace proto;


vec3 to_vec3(aiColor4D& ai_col) {
    return vec3(ai_col.r, ai_col.g, ai_col.b); }

vec3 to_vec3(aiVector3D& ai_vec) {
    return vec3(ai_vec.x, ai_vec.y, ai_vec.z); }


AssetHandle fetch_mesh(StringView filepath) {
    #if 0
    auto& ctx = *proto::context;

    Assimp::Importer importer;
    const aiScene * scene_ptr =
        importer.ReadFile(filepath.str(),
                          aiProcess_CalcTangentSpace      |
                          aiProcess_GenNormals            |
                          aiProcess_Triangulate           |
                          aiProcess_JoinIdenticalVertices |
                          aiProcess_SortByPType);

    if(!scene_ptr) {
        debug_error(debug::category::main, importer.GetErrorString());
        return invalid_asset_handle;
    }

    auto& scene = *scene_ptr;
    Mesh& outmesh_h = create_asset_rref<Mesh>(platform::basename_view(filepath));

    u64 vertex_count = 0;
    u64 index_count = 0;

    for(u32 i=0; i<scene.mNumMeshes; i++) {
        auto& mesh = (*scene.mmeshes[i]);
        index_count += mesh.mNumFaces * 3;
        vertex_count += mesh.mNumVertices;
    }

    outmesh.vertices.resize(vertex_count);
    outmesh.indices.resize(index_count);
    outmesh.spans.resize(scene.mNumMeshes);

    u32 vertices_emited = 0; // index of vertex
    u32 outindex = 0; // index of index basically
    for(u32 m=0; m<scene.mNumMeshes; m++) {
        auto& mesh = (*scene.mmeshes[m]);
        assert(mesh.mnumfaces * 3 > mesh.mNumVertices);
        
        vec3 span_bounds = vec3(0.0, 0.0, 0.0);
        outmesh.spans[m].begin_index = outindex;

        for(u32 f=0; f<mesh.mnumfaces; f++) {
            auto& face = mesh.mfaces[f];
            assert(face.mnumindices == 3);

            for(u32 i=0; i<3; i++) {
                outmesh.indices[outindex + i] = vertices_emited + face.mindices[i];

                auto& outvertex = outmesh.vertices[outmesh.indices[outindex + i]];

                outvertex.position = to_vec3( mesh.mvertices[face.mindices[i]] );

                if(mesh.hasnormals())
                    outvertex.normal =
                        to_vec3( mesh.mnormals[face.mindices[i]] );

                if(mesh.hastexturecoords(0))
                    outvertex.uv =
                        vec2(to_vec3( mesh.mtexturecoords[0][face.mindices[i]]));
                else
                    outvertex.uv = vec2(0.0);

                span_bounds = vec3(max(span_bounds.x, outvertex.position.x),
                                   max(span_bounds.y, outvertex.position.y),
                                   max(span_bounds.z, outvertex.position.z));
            } outindex += 3;
        }
        vertices_emited += mesh.mNumVertices;
        outmesh.spans[m].index_count = outindex - outmesh.spans[m].begin_index;
        outmesh.spans[m].material = ready_materials[mesh.mmaterialindex];

        // render preview of given span
        // hmm, but first you need separate thread...
        // or render per load non realtime
    }
    serialization::save_asset_tree_rec(outmesh_h, outdir);
    #endif
}

