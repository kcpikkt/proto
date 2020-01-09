#pragma once

#include "proto/core/graphics.hh" 
#include "proto/core/util/namespace-shorthands.hh" 
#include "proto/core/util/StringView.hh" 
#include "proto/core/debug/logging.hh" 

#include "stb_image.hh"

using namespace proto;

#if 0 
AssetHandle fetch_texture(StringView filepath) {
    namespace sys = proto::platform;

    log_info(debug::category::data, "Parsing texture ", filepath);

    // init just in case if I wanted to preview loaded texture
    AssetHandle handle =
        create_init_asset<Texture2D>(sys::basename_view(filepath));

    if(!handle) {
        debug_warn(debug::category::data,
                   "Could not create asset for texture ", filepath);
        return handle;
    }

    Texture2D * texture;
    // this cannot fail if create_asset succeded
    // if this fails then there is something seriously wrong with the asset interface
    assert(texture = get_asset<Texture2D>(handle));

    // strings passed to stbi obviously have to be valid cstrings
    if(!filepath.is_cstring()) {
        debug_error(debug::category::main, "filepath StringView is not a cstring, fixme");
        return invalid_asset_handle;
    }

    int x,y,n;
    texture->data = stbi_load(filepath.str(), &x, &y, &n, 0);

    if(!texture->data) {
        log_error(debug::category::main, "Failed to read image data from ", filepath);
        return invalid_asset_handle;
    }

    assert(x); assert(y); assert(n);

    switch(n) {
    case 1:
        texture->format = GL_R;
        texture->gpu_format = GL_R8;
        break;
    case 2:
        texture->format = GL_RG;
        texture->gpu_format = GL_RG8;
        break;
    case 3:
        texture->format = GL_RGB;
        texture->gpu_format = GL_RGB8;
        break;
    case 4:
        texture->format = GL_RGBA;
        texture->gpu_format = GL_RGBA8;
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

#endif
