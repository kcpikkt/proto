#include "proto/proto.hh"
#include "proto/core/platform/common.hh" 
#include "proto/core/graphics/gl.hh" 
#include "proto/core/graphics/rendering.hh" 
#include "proto/core/util/namespace-shorthands.hh" 
#include "proto/core/asset-system/interface.hh" 
#include "proto/core/asset-system/serialization.hh" 

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

#include "assimp/Importer.hpp"
#include "assimp/DefaultLogger.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

using namespace proto;

PROTO_SETUP {
    settings->mode = RuntimeSettings::terminal_mode_bit;
    settings->asset_paths = "res/";
}

vec3 to_vec3(aiColor4D& ai_col) {
    return vec3(ai_col.r, ai_col.g, ai_col.b); }

vec3 to_vec3(aiVector3D& ai_vec) {
    return vec3(ai_vec.x, ai_vec.y, ai_vec.z); }

StringArena texture_paths;
StringView filepath = "res/external/crytek-sponza/sponza.obj";

AssetHandle fetch_texture(StringView rel_path) {
    namespace sys = proto::platform;
    int x,y,n;

    StringView name = sys::basename_view(rel_path);
    // INIT if you want to preview it
    AssetHandle handle =
        create_init_asset<Texture2D>(name);

    if(!handle) {
        debug_warn(debug::category::data,
                   "Could not create asset for texture ", name);
        return handle;
    }

    char _texpath[PROTO_MAX_PATH];
    strview_copy(_texpath, sys::dirname_view(filepath));
    sys::path_ncat(_texpath, rel_path, PROTO_MAX_PATH);

    Texture2D * texture = get_asset<Texture2D>(handle);
    assert(texture);

    texture->data = stbi_load(_texpath, &x, &y, &n, 0);

    assert(x); assert(y); assert(n); assert(texture->data);

    switch(n) {
    case 1:
        texture->format = GL_R8;
        texture->gpu_format = GL_RED;
        break;
    case 3:
        texture->format = GL_RGB8;
        texture->gpu_format = GL_RGB;
        break;
    case 4:
        texture->format = GL_RGBA8;
        texture->gpu_format = GL_RGBA;
        break;
    default:
        debug_warn(debug::category::graphics,
                   "No support for textures with ", n, " channels");
    }

    texture->channels = (u8)n;
    texture->size = ivec2(x,y);

    return handle;
}

PROTO_INIT {
    assert(proto::context);
    auto& ctx = *proto::context;

    texture_paths.init(&ctx.memory);
    stbi_set_flip_vertically_on_load(true);

    assert(filepath.is_cstring());

    Assimp::Importer importer;
    const aiScene * scene_ptr =
        importer.ReadFile(filepath.str(),
                          aiProcess_CalcTangentSpace      |
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
            [&](StringView prop_name) {
                debug_warn(debug::category::main,
                           "Failed to fetch ", prop_name,
                           " value of material ", mat_name, ".");};

        float floattmp;
        if(ai_material.Get(AI_MATKEY_SHININESS, floattmp) == AI_SUCCESS)
            material->shininess = floattmp;
        else
            fetch_fail("shininess");

        aiColor4D colortmp;
        if(ai_material.Get(AI_MATKEY_COLOR_DIFFUSE, colortmp) == AI_SUCCESS)
            material->diffuse_color = to_vec3(colortmp);
        else
            fetch_fail("diffuse_color");

        if(ai_material.Get(AI_MATKEY_COLOR_AMBIENT, colortmp) == AI_SUCCESS)
            material->ambient_color = to_vec3(colortmp);
        else
            fetch_fail("ambient_color");

        if(ai_material.Get(AI_MATKEY_COLOR_SPECULAR, colortmp) == AI_SUCCESS)
            material->specular_color = to_vec3(colortmp);
        else
            fetch_fail("specular_color");

        auto texture_fetch_fail =
            [&](StringView prop_name) {
                debug_warn(debug::category::main,
                           "Failed to fetch texture ", prop_name,
                           " referenced by material ", mat_name, ".");};

        auto warn_multitex = []() {
                debug_warn(debug::category::main,
                           "Multiple textures of the same type "
                           "per mesh are not supported (how?).");};

        aiString texpath; u32 tex_count;

        tex_count = ai_material.GetTextureCount(aiTextureType_DIFFUSE);
        if(tex_count) {
            if(tex_count > 1) warn_multitex();

            ai_material.GetTexture(aiTextureType_DIFFUSE, 0, &texpath);
            if(!texture_paths.contains(texpath.C_Str())) {
                texture_paths.store(texpath.C_Str());

                material->diffuse_map = fetch_texture(texpath.C_Str());
                if(!material->diffuse_map) texture_fetch_fail(texpath.C_Str());

                add_dependency(get_metadata(outmesh_h), material->diffuse_map);
            }
        }

        tex_count = ai_material.GetTextureCount(aiTextureType_AMBIENT);
        if(tex_count) {
            if(tex_count > 1) warn_multitex();

            ai_material.GetTexture(aiTextureType_AMBIENT, 0, &texpath);
            if(!texture_paths.contains(texpath.C_Str())) {
                texture_paths.store(texpath.C_Str());

                material->ambient_map = fetch_texture(texpath.C_Str());
                if(!material->ambient_map) texture_fetch_fail(texpath.C_Str());

                add_dependency(get_metadata(outmesh_h), material->ambient_map);
            }
        }

        tex_count = ai_material.GetTextureCount(aiTextureType_SPECULAR);
        if(tex_count) {
            if(tex_count > 1) warn_multitex();

            ai_material.GetTexture(aiTextureType_SPECULAR, 0, &texpath);
            if(!texture_paths.contains(texpath.C_Str())) {
                texture_paths.store(texpath.C_Str());

                material->specular_map = fetch_texture(texpath.C_Str());
                if(!material->specular_map) texture_fetch_fail(texpath.C_Str());

                add_dependency(get_metadata(outmesh_h), material->specular_map);
            }
        }

        tex_count = ai_material.GetTextureCount(aiTextureType_DISPLACEMENT);
        if(tex_count) {
            if(tex_count > 1) warn_multitex();

            ai_material.GetTexture(aiTextureType_DISPLACEMENT, 0, &texpath);
            if(!texture_paths.contains(texpath.C_Str())) {
                texture_paths.store(texpath.C_Str());

                material->bump_map = fetch_texture(texpath.C_Str());
                if(!material->bump_map) texture_fetch_fail(texpath.C_Str());

                add_dependency(get_metadata(outmesh_h), material->bump_map);
            }
        }
        ready_materials[m] = *material;
    }

    // LOAD MESH GEOMETRY
   
    Mesh& outmesh = *get_asset<Mesh> (outmesh_h);
    u64 vertex_count = 0;
    u64 index_count = 0;

    for(u32 i=0; i<scene.mNumMeshes; i++) {
        auto& mesh = (*scene.mMeshes[i]);
        index_count += mesh.mNumFaces * 3;
        vertex_count += mesh.mNumVertices;
    }

    outmesh.vertices.resize(vertex_count);
    outmesh.indices.resize(index_count);
    outmesh.spans.resize(scene.mNumMeshes);

    u32 vertices_emited = 0; // index of vertex
    u32 outindex = 0; // index of index basically
    for(u32 m=0; m<scene.mNumMeshes; m++) {
        auto& mesh = (*scene.mMeshes[m]);
        assert(mesh.mNumFaces * 3 > mesh.mNumVertices);
        
        vec3 span_bounds = vec3(0.0, 0.0, 0.0);
        outmesh.spans[m].begin_index = outindex;

        for(u32 f=0; f<mesh.mNumFaces; f++) {
            auto& face = mesh.mFaces[f];
            assert(face.mNumIndices == 3);

            for(u32 i=0; i<3; i++) {
                outmesh.indices[outindex + i] = vertices_emited + face.mIndices[i];

                auto& outvertex = outmesh.vertices[outmesh.indices[outindex + i]];

                outvertex.position = to_vec3( mesh.mVertices[face.mIndices[i]] );

                if(mesh.HasNormals())
                    outvertex.normal =
                        to_vec3( mesh.mNormals[face.mIndices[i]] );

                if(mesh.HasTextureCoords(0))
                    outvertex.uv =
                        to_vec3( mesh.mTextureCoords[0][face.mIndices[i]]).xy;
                else
                    outvertex.uv = vec2(0.0);

                span_bounds = vec3(max(span_bounds.x, outvertex.position.x),
                                   max(span_bounds.y, outvertex.position.y),
                                   max(span_bounds.z, outvertex.position.z));
            } outindex += 3;
        }
        vertices_emited += mesh.mNumVertices;
        outmesh.spans[m].index_count = outindex - outmesh.spans[m].begin_index;
        outmesh.spans[m].material = ready_materials[mesh.mMaterialIndex];

        // RENDER PREVIEW OF GIVEN SPAN
        // HMM, but first you need separate thread...
        // or render per load non realtime
    }
    serialization::save_asset_tree_rec(outmesh_h, "outmesh/");
}
