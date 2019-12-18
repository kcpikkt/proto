#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/containers/StringArena.hh"
#include "proto/core/string.hh"

namespace proto {
namespace argparse {

StringView match_sentence(StringView sentences[],
                          s32 sentences_count,
                          s32 sentence_begin_index = 1)
{
    auto& ctx = *context;
    StringView sentence;

    for(s32 s=0; s<sentences_count; s++) {
        StringArena keywords; 
        keywords.init_split(sentences[s], ' ', &context->memory);

        if(ctx.argc - sentence_begin_index >= (s32)keywords.count()) {
            bool valid = true;
            for(s32 w=0; w<(s32)keywords.count(); w++) {
                if( !strview_cmp(keywords[w], ctx.argv[w + sentence_begin_index]) ) {
                    valid = false; break;
                } 
            }
            if(valid) {
                sentence = sentences[s]; break;
            }
        }
    }
    return sentence;
}

} // namespace argparse
} // namespace proto

