#include "fetch.hh"
#include "ar-common.hh"
#include "proto/core/util/namespace-shorthands.hh"

using namespace proto;

AssetHandle fetch(StringView filepath) {
    StringView ext = sys::extension_view(filepath);

    if(ar_flags.at(verbose_bit)) println(filepath);

    for(auto& handler : ext_handlers) {
        if( strview_cmp(ext, handler.ext) ) {
            return handler.proc(filepath);
        }
    }

    log_error(debug::category::main, "Unhandled file extension ", ext);
    return invalid_asset_handle;
}
