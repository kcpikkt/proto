#pragma once

#include "cli-common.hh"
#include "proto/core/asset-system/interface.hh" 
#include "proto/core/asset-system/serialization.hh" 
#include "proto/core/util/namespace-shorthands.hh" 

#include "assimp/Importer.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "proto/core/util/namespace-shorthands.hh" 

using namespace proto;


static vec3 to_vec3(aiColor4D& ai_col) {
    return vec3(ai_col.r, ai_col.g, ai_col.b); }

static vec3 to_vec3(aiVector3D& ai_vec) {
    return vec3(ai_vec.x, ai_vec.y, ai_vec.z); }


static void fetch_model_tree(StringView filepath) {
    #if 0
    assert(proto::context);
    auto& ctx = *proto::context;

    texture_paths.init(&ctx.memory);
    stbi_set_flip_vertically_on_load(true);

    assert(outdir.is_cstring());
    assert(filepath.is_cstring());

    if(!platform::is_directory(outdir)) 
        FAIL(outdir, " is not a directory");

    if(!platform::is_file(filepath)) 
        FAIL(filepath, " is not a file");

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
        return;
    }

    auto& scene = *scene_ptr;
    AssetHandle outmesh_h =
        create_init_asset<Mesh>(platform::basename_view(filepath));

    Array<Material> ready_materials;
    ready_materials.init_resize(scene.mNumMaterials, &context->memory);

    // LOAD MATERIALS AND TEXNAMES
    for(u32 m = 0; m < scene.mNumMaterials; m++) {
        aiMaterial& ai_material = *scene.mMaterials[m];
        StringView mat_name = ai_material.GetName().C_Str();
        AssetHandle material_h = create_asset<Material>(mat_name);
        Material * material = get_asset<Material>(material_h);

        auto fetch_fail =
            [&](stringview prop_name) {
                debug_warn(debug::category::main,
                           "failed to fetch ", prop_name,
                           " value of material ", mat_name, ".");};

        float floattmp;
        if(ai_material.get(ai_matkey_shininess, floattmp) == ai_success)
            material->shininess = floattmp;
        else
            fetch_fail("shininess");

        aicolor4d colortmp;
        if(ai_material.get(ai_matkey_color_diffuse, colortmp) == ai_success)
            material->diffuse_color = to_vec3(colortmp);
        else
            fetch_fail("diffuse_color");

        if(ai_material.get(ai_matkey_color_ambient, colortmp) == ai_success)
            material->ambient_color = to_vec3(colortmp);
        else
            fetch_fail("ambient_color");

        if(ai_material.get(ai_matkey_color_specular, colortmp) == ai_success)
            material->specular_color = to_vec3(colortmp);
        else
            fetch_fail("specular_color");

        auto texture_fetch_fail =
            [&](stringview prop_name) {
                debug_warn(debug::category::main,
                           "failed to fetch texture ", prop_name,
                           " referenced by material ", mat_name, ".");};

        auto warn_multitex = []() {
                debug_warn(debug::category::main,
                           "multiple textures of the same type "
                           "per mesh are not supported (how?).");};

        aistring texpath; u32 tex_count;

        tex_count = ai_material.gettexturecount(aitexturetype_diffuse);
        if(tex_count) {
            if(tex_count > 1) warn_multitex();

            ai_material.gettexture(aitexturetype_diffuse, 0, &texpath);
            if(!texture_paths.contains(texpath.c_str())) {
                texture_paths.store(texpath.c_str());

                material->diffuse_map = fetch_texture(texpath.c_str());
                if(!material->diffuse_map) texture_fetch_fail(texpath.c_str());

                add_dependency(get_metadata(outmesh_h), material->diffuse_map);
            }
        }

        tex_count = ai_material.gettexturecount(aitexturetype_ambient);
        if(tex_count) {
            if(tex_count > 1) warn_multitex();

            ai_material.gettexture(aitexturetype_ambient, 0, &texpath);
            if(!texture_paths.contains(texpath.c_str())) {
                texture_paths.store(texpath.c_str());

                material->ambient_map = fetch_texture(texpath.c_str());
                if(!material->ambient_map) texture_fetch_fail(texpath.c_str());

                add_dependency(get_metadata(outmesh_h), material->ambient_map);
            }
        }

        tex_count = ai_material.gettexturecount(aitexturetype_specular);
        if(tex_count) {
            if(tex_count > 1) warn_multitex();

            ai_material.gettexture(aitexturetype_specular, 0, &texpath);
            if(!texture_paths.contains(texpath.c_str())) {
                texture_paths.store(texpath.c_str());

                material->specular_map = fetch_texture(texpath.c_str());
                if(!material->specular_map) texture_fetch_fail(texpath.c_str());

                add_dependency(get_metadata(outmesh_h), material->specular_map);
            }
        }

        tex_count = ai_material.gettexturecount(aitexturetype_displacement);
        if(tex_count) {
            if(tex_count > 1) warn_multitex();

            ai_material.gettexture(aitexturetype_displacement, 0, &texpath);
            if(!texture_paths.contains(texpath.c_str())) {
                texture_paths.store(texpath.c_str());

                material->bump_map = fetch_texture(texpath.c_str());
                if(!material->bump_map) texture_fetch_fail(texpath.c_str());

                add_dependency(get_metadata(outmesh_h), material->bump_map);
            }
        }

        tex_count = ai_material.gettexturecount(aitexturetype_opacity);
        if(tex_count) {
            if(tex_count > 1) warn_multitex();

            ai_material.gettexture(aitexturetype_opacity, 0, &texpath);
            if(!texture_paths.contains(texpath.c_str())) {
                texture_paths.store(texpath.c_str());

                material->opacity_map = fetch_texture(texpath.c_str());
                if(!material->opacity_map) texture_fetch_fail(texpath.c_str());
                material->transparency = true;

                add_dependency(get_metadata(outmesh_h), material->opacity_map);
            }
        }

        ready_materials[m] = *material;
    }

    // load mesh geometry
   
    mesh& outmesh = *get_asset<mesh> (outmesh_h);
    u64 vertex_count = 0;
    u64 index_count = 0;

    for(u32 i=0; i<scene.mnummeshes; i++) {
        auto& mesh = (*scene.mmeshes[i]);
        index_count += mesh.mnumfaces * 3;
        vertex_count += mesh.mnumvertices;
    }

    outmesh.vertices.resize(vertex_count);
    outmesh.indices.resize(index_count);
    outmesh.spans.resize(scene.mnummeshes);

    u32 vertices_emited = 0; // index of vertex
    u32 outindex = 0; // index of index basically
    for(u32 m=0; m<scene.mnummeshes; m++) {
        auto& mesh = (*scene.mmeshes[m]);
        assert(mesh.mnumfaces * 3 > mesh.mnumvertices);
        
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
        vertices_emited += mesh.mnumvertices;
        outmesh.spans[m].index_count = outindex - outmesh.spans[m].begin_index;
        outmesh.spans[m].material = ready_materials[mesh.mmaterialindex];

        // render preview of given span
        // hmm, but first you need separate thread...
        // or render per load non realtime
    }
    serialization::save_asset_tree_rec(outmesh_h, outdir);
    #endif
}

