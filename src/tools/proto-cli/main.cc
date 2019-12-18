#include "proto/proto.hh"
#include "proto/core/platform/common.hh" 
#include "proto/core/graphics/gl.hh" 
#include "proto/core/graphics/rendering.hh" 
#include "proto/core/util/namespace-shorthands.hh" 
#include "proto/core/asset-system/interface.hh" 
#include "proto/core/asset-system/serialization.hh" 
#include "proto/core/util/argparse.hh" 

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

StringView dirpath = "";
StringView basedir = "";

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
    strview_copy(_texpath, basedir);
    sys::path_ncat(_texpath, rel_path, PROTO_MAX_PATH);

    if(!platform::is_file(_texpath)) {
        debug_error(debug::category::data,
                    _texpath, " is not a file");
        return invalid_asset_handle;
    }

    Texture2D * texture = get_asset<Texture2D>(handle);
    assert(texture);

    log_info(debug::category::data, "fetching texture ", _texpath);

    texture->data = stbi_load(_texpath, &x, &y, &n, 0);

    assert(x); assert(y); assert(n); assert(texture->data);

    switch(n) {
    case 1:
        texture->format = GL_R8;
        texture->gpu_format = GL_RED;
        break;
    case 2:
        texture->format = GL_RG8;
        texture->gpu_format = GL_RG;
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
        return invalid_asset_handle;
    }

    texture->channels = (u8)n;
    texture->size = ivec2(x,y);

    return handle;
}

#define LEN(arr) (sizeof(arr)/sizeof(arr[0]))

#define FAIL(...) { \
    debug_error(debug::category::main, __VA_ARGS__); return;}

StringView cmdline_sentences[] = {"parse mesh",
                                  "parse cubemap",
};

void parse_mesh(StringView filepath, StringView outdir);
void parse_cubemap(StringView, StringView, StringView,
                   StringView, StringView, StringView,
                   StringView);

PROTO_INIT {
    assert(proto::context);
    auto& ctx = *proto::context;

    StringView sentence =
        argparse::match_sentence(cmdline_sentences, LEN(cmdline_sentences), 2);

    if(sentence) {
        /*  */ if(strview_cmp(sentence, "parse mesh")) {
        log_info(debug::category::main, "Mesh parsing");
            if(ctx.argc != 6) {
                log_error(debug::category::main,
                          "parse mesh [mesh file] [out directory]");
            } else {
                dirpath = ctx.argv[5];
                basedir = sys::dirname_view(ctx.argv[4]);

                parse_mesh(ctx.argv[4], ctx.argv[5]);
            }
            return;
        } else if(strview_cmp(sentence, "parse cubemap")) {
        log_info(debug::category::main, "Cubemap parsing");
            if(ctx.argc != 12) {
                log_info(debug::category::main,
                        "parse cubemap [directory] "
                         "[right] [left] [up] [down] [forward [back] [out]");
            } else { 
                dirpath = ctx.argv[4];

                if(!platform::is_directory(dirpath)) 
                    FAIL(dirpath, " is not a directory");
   
                parse_cubemap(ctx.argv[5], ctx.argv[6], ctx.argv[7],
                              ctx.argv[8], ctx.argv[9], ctx.argv[10],
                              ctx.argv[11]);
            }
            return;
        }
    } else {
        log_error(debug::category::main, "No task specified, exiting.");
        return;
    }
}


void parse_mesh(StringView filepath, StringView outdir) {
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

        tex_count = ai_material.GetTextureCount(aiTextureType_OPACITY);
        if(tex_count) {
            if(tex_count > 1) warn_multitex();

            ai_material.GetTexture(aiTextureType_OPACITY, 0, &texpath);
            if(!texture_paths.contains(texpath.C_Str())) {
                texture_paths.store(texpath.C_Str());

                material->opacity_map = fetch_texture(texpath.C_Str());
                if(!material->opacity_map) texture_fetch_fail(texpath.C_Str());
                material->transparency = true;

                add_dependency(get_metadata(outmesh_h), material->opacity_map);
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
    serialization::save_asset_tree_rec(outmesh_h, outdir);
}

void parse_cubemap(StringView right, StringView left,
                   StringView up, StringView down,
                   StringView forward, StringView back,
                   StringView out)
{

    //    char rt_filepath[PROTO_MAX_PATH]; strview_copy(rt_filepath, dirpath);
    //    platform::path_ncat(rt_filepath, right, PROTO_MAX_PATH);
    //    char lf_filepath[PROTO_MAX_PATH]; strview_copy(lf_filepath, dirpath);
    //    platform::path_ncat(lf_filepath, left, PROTO_MAX_PATH);
    //    char up_filepath[PROTO_MAX_PATH]; strview_copy(up_filepath, dirpath);
    //    platform::path_ncat(up_filepath, up, PROTO_MAX_PATH);
    //    char dn_filepath[PROTO_MAX_PATH]; strview_copy(dn_filepath, dirpath);
    //    platform::path_ncat(dn_filepath, down, PROTO_MAX_PATH);
    //    char fw_filepath[PROTO_MAX_PATH]; strview_copy(fw_filepath, dirpath);
    //    platform::path_ncat(fw_filepath, forward, PROTO_MAX_PATH);
    //    char bk_filepath[PROTO_MAX_PATH]; strview_copy(bk_filepath, dirpath);
    //    platform::path_ncat(bk_filepath, back, PROTO_MAX_PATH);
    //
    //    if(!platform::is_file(rt_filepath)) FAIL(rt_filepath, " is not a file");
    //    if(!platform::is_file(lf_filepath)) FAIL(lf_filepath, " is not a file");
    //    if(!platform::is_file(up_filepath)) FAIL(up_filepath, " is not a file");
    //    if(!platform::is_file(dn_filepath)) FAIL(dn_filepath, " is not a file");
    //    if(!platform::is_file(fw_filepath)) FAIL(fw_filepath, " is not a file");
    //    if(!platform::is_file(bk_filepath)) FAIL(bk_filepath, " is not a file");

    auto rt_tex_h = fetch_texture(right);
    auto lf_tex_h = fetch_texture(left);
    auto up_tex_h = fetch_texture(up);
    auto dn_tex_h = fetch_texture(down);
    auto fw_tex_h = fetch_texture(forward);
    auto bk_tex_h = fetch_texture(back);

    if(!rt_tex_h) FAIL("Could not load ", right);
    if(!lf_tex_h) FAIL("Could not load ", left);
    if(!up_tex_h) FAIL("Could not load ", up);
    if(!dn_tex_h) FAIL("Could not load ", down);
    if(!fw_tex_h) FAIL("Could not load ", forward);
    if(!bk_tex_h) FAIL("Could not load ", back);

    Cubemap& cubemap = create_asset_rref<Cubemap>(platform::basename_view(right));

    Texture2D& ref = get_asset_ref<Texture2D>(rt_tex_h);
    cubemap.init(ref.size, ref.format, ref.gpu_format, ref.datatype);
    cubemap.channels = ref.channels;

    cubemap.data[0] = get_asset_ref<Texture2D>(rt_tex_h).data;
    cubemap.data[1] = get_asset_ref<Texture2D>(lf_tex_h).data;
    cubemap.data[2] = get_asset_ref<Texture2D>(up_tex_h).data;
    cubemap.data[3] = get_asset_ref<Texture2D>(dn_tex_h).data;
    cubemap.data[4] = get_asset_ref<Texture2D>(fw_tex_h).data;
    cubemap.data[5] = get_asset_ref<Texture2D>(bk_tex_h).data;

    serialization::save_asset(&cubemap, out);
}
