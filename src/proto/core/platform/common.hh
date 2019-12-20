#pragma once
#include "proto/core/platform/macros.hh"
#include "proto/core/platform/RuntimeSettings.hh"
#include "proto/core/common.hh"

namespace proto {
    struct Context;
    extern Context * context;

namespace platform {
    static const char * proto_banner = R"(
    _|_|_|    _|  _|_|    _|_|    _|_|_|_|    _|_|
    _|    _|  _|_|      _|    _|    _|      _|    _|
    _|    _|  _|        _|    _|    _|      _|    _|
    _|_|_|    _|          _|_|        _|_|    _|_|
    _|
    _|)";

    static const char * indent = "    ";
    static const char * separator =
"    ------------------------------------------------";

#define PROTO_MAX_PATH 256

#define PROTO_EXTERNAL extern "C"

// SET CONTEXT
// context is passed through that function to the client library
// *******************************************************************
#define PROTO_CLIENT_SET_CONTEXT_FUNCTION proto_client_set_context
#define PROTO_CLIENT_SET_CONTEXT_FUNCTION_NAME PROTO_STR(PROTO_CLIENT_SET_CONTEXT_FUNCTION)

#define PROTO_CLIENT_SET_CONTEXT_FUNCTION_SIGNATURE(NAME)  \
    void NAME(proto::Context * host_context)

typedef PROTO_CLIENT_SET_CONTEXT_FUNCTION_SIGNATURE(ClientSetContextFunction);

#define PROTO_SET_CONTEXT                                       \
    PROTO_EXTERNAL PROTO_CLIENT_SET_CONTEXT_FUNCTION_SIGNATURE \
(PROTO_CLIENT_SET_CONTEXT_FUNCTION)

#define PROTO_SET_CONTEXT_DEFINITION \
    PROTO_SET_CONTEXT { proto::context = host_context; }

#define PROTO_CONTEXT \
    namespace proto { Context * context = nullptr; }                    \
    PROTO_SET_CONTEXT_DEFINITION

/*
Set of functions to implement by the client
 */
// SETUP
// *******************************************************************

#define PROTO_CLIENT_SETUP_FUNCTION proto_client_setup
#define PROTO_CLIENT_SETUP_FUNCTION_NAME PROTO_STR(PROTO_CLIENT_SETUP_FUNCTION)

#define PROTO_CLIENT_SETUP_FUNCTION_SIGNATURE(NAME)  \
    void NAME([[maybe_unused]]proto::RuntimeSettings * settings)

typedef PROTO_CLIENT_SETUP_FUNCTION_SIGNATURE(ClientSetupFunction);

#define PROTO_SETUP                                       \
    PROTO_EXTERNAL PROTO_CLIENT_SETUP_FUNCTION_SIGNATURE \
(PROTO_CLIENT_SETUP_FUNCTION)

// INIT
// *******************************************************************
#define PROTO_CLIENT_INIT_FUNCTION proto_client_init
#define PROTO_CLIENT_INIT_FUNCTION_NAME PROTO_STR(PROTO_CLIENT_INIT_FUNCTION)

#define PROTO_CLIENT_INIT_FUNCTION_SIGNATURE(NAME)  \
    void NAME()

typedef PROTO_CLIENT_INIT_FUNCTION_SIGNATURE(ClientInitFunction);

#define PROTO_INIT                                       \
    PROTO_EXTERNAL PROTO_CLIENT_INIT_FUNCTION_SIGNATURE \
(PROTO_CLIENT_INIT_FUNCTION)

// UPDATE
// *******************************************************************
#define PROTO_CLIENT_UPDATE_FUNCTION proto_client_update
#define PROTO_CLIENT_UPDATE_FUNCTION_NAME PROTO_STR(PROTO_CLIENT_UPDATE_FUNCTION)

#define PROTO_CLIENT_UPDATE_FUNCTION_SIGNATURE(NAME) \
    void NAME()

typedef PROTO_CLIENT_UPDATE_FUNCTION_SIGNATURE(ClientUpdateFunction);

#define PROTO_UPDATE                                       \
    PROTO_EXTERNAL PROTO_CLIENT_UPDATE_FUNCTION_SIGNATURE \
(PROTO_CLIENT_UPDATE_FUNCTION)

// LINK
// *******************************************************************
#define PROTO_CLIENT_LINK_FUNCTION proto_client_link
#define PROTO_CLIENT_LINK_FUNCTION_NAME PROTO_STR(PROTO_CLIENT_LINK_FUNCTION)

#define PROTO_CLIENT_LINK_FUNCTION_SIGNATURE(NAME)  \
    void NAME()

typedef PROTO_CLIENT_LINK_FUNCTION_SIGNATURE(ClientLinkFunction);

#define PROTO_LINK                                       \
    PROTO_EXTERNAL PROTO_CLIENT_LINK_FUNCTION_SIGNATURE \
(PROTO_CLIENT_LINK_FUNCTION)

// UNLINK
// *******************************************************************
#define PROTO_CLIENT_UNLINK_FUNCTION proto_client_unlink
#define PROTO_CLIENT_UNLINK_FUNCTION_NAME PROTO_STR(PROTO_CLIENT_UNLINK_FUNCTION)

#define PROTO_CLIENT_UNLINK_FUNCTION_SIGNATURE(NAME)  \
    void NAME()

typedef PROTO_CLIENT_UNLINK_FUNCTION_SIGNATURE(ClientUnlinkFunction);

#define PROTO_UNLINK                                       \
    PROTO_EXTERNAL PROTO_CLIENT_UNLINK_FUNCTION_SIGNATURE \
(PROTO_CLIENT_UNLINK_FUNCTION)

// CLOSE
// *******************************************************************
#define PROTO_CLIENT_CLOSE_FUNCTION proto_client_close
#define PROTO_CLIENT_CLOSE_FUNCTION_NAME PROTO_STR(PROTO_CLIENT_CLOSE_FUNCTION)

#define PROTO_CLIENT_CLOSE_FUNCTION_SIGNATURE(NAME)  \
    void NAME()

typedef PROTO_CLIENT_CLOSE_FUNCTION_SIGNATURE(ClientCloseFunction);

#define PROTO_CLOSE                                       \
    PROTO_EXTERNAL PROTO_CLIENT_CLOSE_FUNCTION_SIGNATURE \
(PROTO_CLIENT_CLOSE_FUNCTION)

} // namespace platform
} // namespace proto


