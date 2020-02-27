#pragma once
#include "proto/core/common.hh"

namespace proto {

// Not particularly sophisticated error handling but, having tried many things,
// this is so far the simplest yet most effective error handling method I found.
// KISS FTW
typedef u32 Err;

enum : Err {
    SUCCESS = 0,
    DBG_ERR,
    UNIMPL_ERR,
    ///////////////////
    INVALID_ARG_ERR,
    ///////////////////
    MEM_ERR,
    MEM_MAP_ERR,
    MEM_ALLOC_ERR,
    MEM_OOM_ERR,
    ///////////////////// 
    IO_ERR,
    IO_FILE_ERR,
    IO_FILE_OPEN_ERR,
    IO_FILE_CLOSE_ERR,
    IO_FILE_READ_ERR,
    IO_FILE_SEEK_ERR,
    IO_FILE_WRITE_ERR,
    IO_FILE_RESERVE_ERR,
    IO_FILE_MAP_ERR,
    /////////////////////
    ST_ERR,
    ST_DBL_DEEP_DTOR_ERR,
    ST_DBL_SHALLOW_DTOR_ERR,
    ST_DTOR_UNINIT_ERR,
    ST_OUT_OF_SCOPE_LEAK_ERR,
    ST_DEEP_DTOR_UNIMPL_ERR,
    ST_DTOR_ERR,
    /////////////////////
    AR_ERR,
    AR_INVALID_ERR,
    AR_NO_FREE_NODES_ERR,
    AR_NO_BLOCK_SPAN_ERR,
    /////////////////////
    _err_count
};

// NOTE(kacper): Why it is a pair instead of only a message you may ask;
// well, since C++ does not provide any enum to value mapping system
// it is possible that I may make mistake while adding new error codes and messages
// and error code will not correspond to with the index of appropriate message.
// Having pairs of code-message I can assert in advance that message at the given
// index correspond with the appropriate error code.
//
#define _PROTO_DEF_ERRMSG_OVERLOAD1(CODE) {CODE, PROTO_STR(CODE)}
#define _PROTO_DEF_ERRMSG_OVERLOAD2(CODE, MSG) {CODE, PROTO_STR(CODE) ": " MSG}

#define _PROTO_DEF_ERRMSG_OVERLOAD(_1, _2, NAME, ...) NAME
#define _PROTO_DEF_ERRMSG(...) \
    _PROTO_DEF_ERRMSG_OVERLOAD \
    (__VA_ARGS__, _PROTO_DEF_ERRMSG_OVERLOAD2, _PROTO_DEF_ERRMSG_OVERLOAD1 ) \
    (__VA_ARGS__)


struct _ErrMessage { Err ec; const char * msg = "No error message yet."; };
constexpr static _ErrMessage _err_msg_map[] = 
{
    _PROTO_DEF_ERRMSG(SUCCESS),
    _PROTO_DEF_ERRMSG(DBG_ERR,
                      "Sanity check failed, there is a problem with integrity of the framework code."),
    _PROTO_DEF_ERRMSG(UNIMPL_ERR,
                      "This error should have its own error code but doesn't, implement it!."),
    _PROTO_DEF_ERRMSG(INVALID_ARG_ERR,
                      "Argument passed to a function was invalid."),
    _PROTO_DEF_ERRMSG(MEM_ERR),
    _PROTO_DEF_ERRMSG(MEM_MAP_ERR,
                      "Failed to map memory."),
    _PROTO_DEF_ERRMSG(MEM_ALLOC_ERR,
                      "Failed to allocate memory."),
    _PROTO_DEF_ERRMSG(MEM_OOM_ERR,
                      "Out of memory."),
    _PROTO_DEF_ERRMSG(IO_ERR,
                      "Input/Output error."),
    _PROTO_DEF_ERRMSG(IO_FILE_ERR),
    _PROTO_DEF_ERRMSG(IO_FILE_OPEN_ERR),
    _PROTO_DEF_ERRMSG(IO_FILE_CLOSE_ERR),
    _PROTO_DEF_ERRMSG(IO_FILE_READ_ERR),
    _PROTO_DEF_ERRMSG(IO_FILE_SEEK_ERR),
    _PROTO_DEF_ERRMSG(IO_FILE_WRITE_ERR),
    _PROTO_DEF_ERRMSG(IO_FILE_RESERVE_ERR),
    _PROTO_DEF_ERRMSG(IO_FILE_MAP_ERR),
    _PROTO_DEF_ERRMSG(ST_ERR,
                      "Error related to object state"),
    _PROTO_DEF_ERRMSG(ST_DBL_DEEP_DTOR_ERR,
                      "Deep destructor called two times on stateful object."),
    _PROTO_DEF_ERRMSG(ST_DBL_SHALLOW_DTOR_ERR,
                      "Shallow destructor called two times on stateful object."),
    _PROTO_DEF_ERRMSG(ST_DTOR_UNINIT_ERR,
                      "Destructor called on uninitialized stateful object."),
    _PROTO_DEF_ERRMSG(ST_OUT_OF_SCOPE_LEAK_ERR,
                      "No destruction performed on initialized stateful object "
                      "going out of scope, possible leak."),
    _PROTO_DEF_ERRMSG(ST_DEEP_DTOR_UNIMPL_ERR,
                      "Deep destructor not implemented in stateful type."),
    _PROTO_DEF_ERRMSG(ST_DTOR_ERR,
                      "Error occured in destructor function."),
    _PROTO_DEF_ERRMSG(AR_ERR),
    _PROTO_DEF_ERRMSG(AR_INVALID_ERR,
                      "Archive file is invalid or corrupted."),
    _PROTO_DEF_ERRMSG(AR_NO_FREE_NODES_ERR,
                      "No free nodes left in an archive."),
    _PROTO_DEF_ERRMSG(AR_NO_BLOCK_SPAN_ERR,
                      "No sufficient free blocks span left in an archive."),
};

constexpr static const char * errmsg(Err code) { 
    if(code >= count_of(_err_msg_map))
        return "Unknown error.";

    assert(_err_msg_map[code].ec == code);
    return _err_msg_map[code].msg;
}

constexpr static Err _errmsg_sanity_check() {
    if(count_of(_err_msg_map) != _err_count)
        return DBG_ERR;

    for(u64 i=0; i<count_of(_err_msg_map); ++i)
        if(_err_msg_map[i].ec != i) return DBG_ERR;

    return SUCCESS;
}

} // namespace proto
