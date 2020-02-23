#include "fetch_image.hh"
#include "ar-common.hh"
#include "proto/core/asset-system/interface.hh"

#include "stb_image.hh"

using namespace proto;

AssetHandle fetch_image(StringView filepath) {

    auto& texture = create_asset_rref<Texture2D>(sys::basename_view(filepath));

    // strings passed to stbi have to be valid cstrings
    if(!filepath.is_cstring()) {
        debug_error(debug::category::main, "filepath StringView is not a cstring, fixme");
        return invalid_asset_handle;
    }

    int x,y,n;
    void * data = stbi_load(filepath.str, &x, &y, &n, 0);

    if(!data) {
        log_error(debug::category::main, "Failed to read image data from ", filepath);
        return invalid_asset_handle;
    }
    assert(x); assert(y); assert(n);

    switch(n) {
    case 1:
        texture.format = GL_R;
        texture.gpu_format = GL_R8;
        break;
    case 2:
        texture.format = GL_RG;
        texture.gpu_format = GL_RG8;
        break;
    case 3:
        texture.format = GL_RGB;
        texture.gpu_format = GL_RGB8;
        break;
    case 4:
        texture.format = GL_RGBA;
        texture.gpu_format = GL_RGBA8;
        break;
    default:
        debug_warn(debug::category::graphics,
                   "No support for textures with ", n, " channels");
        return invalid_asset_handle;
    }

    texture.gl_datatype = GL_UNSIGNED_BYTE; //right?
    texture.channels = (u8)n;
    texture.size = ivec2(x,y);

    auto header = serialization::AssetHeader<Texture2D>(texture);
    
    texture.cached = context->memory.alloc_buf(header.datasize);
    allocated_buffers.push_back(texture.cached);
    assert(texture.cached);

    memcpy(texture.cached.data8                          , &header, sizeof(header));
    memcpy(texture.cached.data8 + header.pixeldata_offset, data   , header.pixeldata_size);

    texture.flags.set(Texture2D::cached_bit);

    stbi_image_free(data);
 
    return texture.handle;
}

