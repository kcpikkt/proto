#include "fetch.hh"

int fetch(StringView filepath) {
    StringView ext = sys::extension_view(filepath);

    for(auto& handler : ext_handlers) {
        if( strview_cmp(ext, handler.ext) ) {
            handler.proc(filepath);
            return 0;
        }
    }

    log_error(debug::category::main, "Unhandled file extension ", ext);
    return -1;
}
