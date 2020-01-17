#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/containers/StringArena.hh"
#include "proto/core/string.hh"
#include "proto/core/util/String.hh"

namespace proto {
namespace argparse {

struct Option {
    const char * name;
    char shorthand;

    constexpr static u8 required_bit = BIT(0); // this option is absolutely requiered
    constexpr static u8 param_bit = BIT(1);  // this option takes parameter
    constexpr static u8 multiple_bit = BIT(2); // this option can appear multiple times

    Bitfield<u8> flags;
};

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


int match_options(const Option options[],
                  s32 options_count,
                  ArrayMap<u64, StringView> & matched,
                  Array<StringView> & rest)
{
    auto& ctx = *context;

    auto try_match =
        [](const char * arg, const Option& option) {
            s32 len = strlen(arg);
            if(arg[0] != '-' || len<2) return false;

            if(  arg[1] == option.shorthand ||
                (arg[1] == '-' && strcmp(option.name, &arg[2]) == 0)) return true;
            else
                return false;
        };

    for(s32 i=0; i<ctx.argc; ++i) {
        const char * arg = ctx.argv[i];

        bool opt_match = false; 

        for(s32 j=0; j<options_count; ++j) {
            auto& opt = options[j];

            if(try_match(arg, opt)) {

                opt_match = true;
                if(opt.flags.check(Option::param_bit)) {

                    if(i == ctx.argc) {
                        log_error(debug::category::main, "Option ", arg, " requires a paramter.");
                        return -1;
                    } else {
                        if(!opt.flags.check(Option::multiple_bit) && matched.contains_key(j)) {
                            log_error(debug::category::main, "Multiple options --", opt.name, " specified.");
                            return -1;
                        }
                        matched.push_back(j, ctx.argv[++i]);
                    }
                } else
                    matched.push_back(j, StringView());

            }
        }

        if(!opt_match) rest.push_back(arg);
    }

    for(s64 i=0; i<options_count; ++i) {
        if( options[i].flags.check(Option::required_bit) && !matched.contains_key(i) )
        {
            log_error(debug::category::main, "Option --", options[i].name, " is required.");
            return -1;
        }
    }

    return 0;
}

} // namespace argparse
} // namespace proto

