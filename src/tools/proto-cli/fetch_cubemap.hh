#pragma once

#include "proto/core/graphics.hh" 
#include "proto/core/util/namespace-shorthands.hh" 
#include "proto/core/util/StringView.hh" 
#include "proto/core/debug/logging.hh" 

#include "fetch_texture.hh"

using namespace proto;

AssetHandle fetch_cubemap(StringView right,   StringView left,
                          StringView up,      StringView down,
                          StringView forward, StringView back)
{
    log_info(debug::category::data, "Parsing cubemap");

    auto rt_tex_h = fetch_texture(right);
    auto lf_tex_h = fetch_texture(left);
    auto up_tex_h = fetch_texture(up);
    auto dn_tex_h = fetch_texture(down);
    auto fw_tex_h = fetch_texture(forward);
    auto bk_tex_h = fetch_texture(back);

    auto try_fail = [](AssetHandle handle, StringView texpath) {
                        if(!handle) {
                            log_error(debug::category::data, "Failed to parse texture ", texpath);
                            return 1;
                        } else
                            return 0;};

    if(try_fail(rt_tex_h, right)   ||
       try_fail(lf_tex_h, left)    ||
       try_fail(up_tex_h, up)      ||
       try_fail(dn_tex_h, down)    ||
       try_fail(fw_tex_h, forward) ||
       try_fail(bk_tex_h, back))
    {
        log_info(debug::category::data, "Could not create cubemap");
        return invalid_asset_handle;
    }

    AssetHandle handle =
        create_init_asset<Cubemap>(sys::basename_view(right));

    if(!handle) {
        debug_warn(debug::category::data, "Could not create asset for cubemap");
        return handle;
    }

    Cubemap * cubemap;
    // this cannot fail if create_asset succeded
    // if this fails then there is something seriously wrong with the asset interface
    assert(cubemap = get_asset<Cubemap>(handle));

    Texture2D& ref = get_asset_ref<Texture2D>(rt_tex_h);

    cubemap->size       = ref.size;
    cubemap->format     = ref.format;
    cubemap->gpu_format = ref.gpu_format;
    cubemap->datatype   = ref.datatype;
    cubemap->channels   = ref.channels;

    cubemap->data[0] = get_asset_ref<Texture2D>(rt_tex_h).data;
    cubemap->data[1] = get_asset_ref<Texture2D>(lf_tex_h).data;
    cubemap->data[2] = get_asset_ref<Texture2D>(up_tex_h).data;
    cubemap->data[3] = get_asset_ref<Texture2D>(dn_tex_h).data;
    cubemap->data[4] = get_asset_ref<Texture2D>(fw_tex_h).data;
    cubemap->data[5] = get_asset_ref<Texture2D>(bk_tex_h).data;
       
    return handle;
}
