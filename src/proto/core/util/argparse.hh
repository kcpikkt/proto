#pragma once
#include "proto/core/common/types.hh"
#include "proto/core/context.hh"
#include "proto/core/containers/ArrayMap.hh"
#include "proto/core/string.hh"

namespace proto {
    //    #if 0
    //namespace argparse {
    //
    //struct CmdOpt {
    //    const char * name;
    //    char shorthand;
    //
    //    constexpr static u8 required_bit = BIT(0); // this option is absolutely requiered
    //    constexpr static u8 param_bit = BIT(1);  // this option takes parameter
    //    constexpr static u8 multiple_bit = BIT(2); // this option can appear multiple times
    //    constexpr static u8 pos_1 = BIT(2); // this option can appear multiple times
    //
    //    Bitfield<u8> flags;
    //};
    //
    //StringView match_sentence(StringView sentences[],
    //                          s32 sentences_count,
    //                          s32 sentence_begin_index = 1)
    //{
    //    auto& ctx = *context;
    //    StringView sentence;
    //
    //    for(s32 s=0; s<sentences_count; s++) {
    //        StringArena keywords; 
    //        keywords.init_split(sentences[s], ' ', &context->memory);
    //
    //        if(ctx.argc - sentence_begin_index >= (s32)keywords.count()) {
    //            bool valid = true;
    //            for(s32 w=0; w<(s32)keywords.count(); w++) {
    //                if( !strview_cmp(keywords[w], ctx.argv[w + sentence_begin_index]) ) {
    //                    valid = false; break;
    //                } 
    //            }
    //            if(valid) {
    //                sentence = sentences[s]; break;
    //            }
    //        }
    //    }
    //    return sentence;
    //}
    //
    //
    //int match_options(const CmdOpt options[],
    //                  s32 options_count,
    //                  ArrayMap<u64, StringView> & matched,
    //                  Array<StringView> & rest)
    //{
    //    auto& ctx = *context;
    //
    //    auto try_match =
    //        [](const char * arg, const CmdOpt& option) {
    //            s32 len = strlen(arg);
    //            if(arg[0] != '-' || len<2) return false;
    //
    //            if(  arg[1] == option.shorthand ||
    //                (arg[1] == '-' && strcmp(option.name, &arg[2]) == 0)) return true;
    //            else
    //                return false;
    //        };
    //
    //    for(s32 i=0; i<ctx.argc; ++i) {
    //        const char * arg = ctx.argv[i];
    //
    //        bool opt_match = false; 
    //
    //        for(s32 j=0; j<options_count; ++j) {
    //            auto& opt = options[j];
    //
    //            if(try_match(arg, opt)) {
    //
    //                opt_match = true;
    //                if(opt.flags.check(CmdOpt::param_bit)) {
    //
    //                    if(i == ctx.argc) {
    //                        log_error(debug::category::main, "CmdOpt ", arg, " requires a paramter.");
    //                        return -1;
    //                    } else {
    //                        if(!opt.flags.check(CmdOpt::multiple_bit) && matched.contains_key(j)) {
    //                            log_error(debug::category::main, "Multiple options --", opt.name, " specified.");
    //                            return -1;
    //                        }
    //                        matched.push_back(j, ctx.argv[++i]);
    //                    }
    //                } else
    //                    matched.push_back(j, StringView());
    //
    //            }
    //        }
    //
    //        if(!opt_match) rest.push_back(arg);
    //    }
    //
    //    for(s64 i=0; i<options_count; ++i) {
    //        if( options[i].flags.check(CmdOpt::required_bit) && !matched.contains_key(i) )
    //        {
    //            log_error(debug::category::main, "CmdOpt --", options[i].name, " is required.");
    //            return -1;
    //        }
    //    }
    //
    //    return 0;
    //}
    //
    //} // namespace argparse
    //    #endif

struct CmdOpt {
    StringView name, alt_name = "", desc = "";
};

template<size_t N>
int _argparse_sanity_check(CmdOpt (&opts)[N]) {
    for(u64 i=0; i<count_of(opts); ++i) {
        auto& opt = opts[i];
        if( (!opt.name || !opt.name.length) && (!opt.alt_name || !opt.alt_name.length) ) {
            debug_error(debug::category::main, "CmdOpt with no name/alt_name.");
            return -1;
        }

        if( strview_cmp(opt.name, opt.alt_name) ) {
            debug_error(debug::category::main,
                "CmdOpt has the same name as alt_name (alt_name is not mandatory).");
            return -1;
        }
    }
    // check for doubles
    for(u64 i=0; i<count_of(opts)-1; ++i) {
        for(u64 j=i+1; j<count_of(opts); ++j) {
            auto& fst = opts[i], snd = opts[j];

            if(fst.name && fst.name.length) {
                if( strview_cmp(fst.name, snd.name) || strview_cmp(fst.name, snd.alt_name))
                {
                    debug_error(debug::category::main, "Two options with the same name/alt_name.");
                    return -2;
                }
            }

            if(fst.alt_name && fst.alt_name.length) {
                if( strview_cmp(fst.alt_name, snd.name) || strview_cmp(fst.alt_name, snd.alt_name))
                {
                    debug_error(debug::category::main, "Two options with the same name/alt_name.");
                    return -2;
                }
            }
        }
    }
    return 0;
}

using CmdOptArgMap = ArrayMap<StringView, Array<StringView>>;

template<size_t N>
int argparse(CmdOptArgMap& optarg_map, CmdOpt (&opts)[N], char hyp = '-') {
    auto& ctx = *context;
    if(_argparse_sanity_check(opts)) return -1;

    CmdOpt * active_opt;

    for(u64 i=0; i<ctx.cmdline.count(); ++i) {
        auto arg = ctx.cmdline[i];

        assert(arg && arg.length);

        if(arg[0] == hyp) {
            arg.trim_prefix(1);

            bool match = false;
            for(auto opt : opts) {
                if( strview_cmp(arg, opt.name) || strview_cmp(arg, opt.alt_name) ) {
                    optarg_map.push_back(opt.name, Array<StringView>() );
                    optarg_map.back().init(optarg_map.values._allocator);

                    active_opt = &opt;

                    match = true;
                }
            }
            if(!match) {
                log_error(debug::category::main,
                          "Invalid cmdline option ", ctx.cmdline[i]);
                return -1;
            }
        } else {
            if(!optarg_map.count() || !active_opt) {
                log_error(debug::category::main,
                          "Invalid cmdline option ", ctx.cmdline[i]);
                return -1;
            } else
                optarg_map.back().push_back(ctx.cmdline[i]);
        }
    }
    return 0;
}

template<size_t N>
int argparse_print_help(CmdOpt (&opts)[N], char hyp = '-') {
    u64 max_name_len = 0, max_alt_name_len = 0;
    for(auto opt : opts)
        max_name_len = max(max_name_len, opt.name.length + 1);

    for(auto opt : opts)
        max_alt_name_len = max(max_alt_name_len, opt.alt_name.length + 1);

    u64 len;
    for(auto opt : opts) {
        printn(' ', 4);

        if(opt.name.length) {
            len = opt.name.length + 1;
            print(hyp, opt.name);
        } else
            len = 0;
        printn(' ', max_name_len - len);


        if(opt.alt_name.length) {
            print(", ");
            len = opt.alt_name.length + 1;
            print(hyp, opt.alt_name);
        } else {
            print("  ");
            len = 0;
        }

        printn(' ', max_alt_name_len - len);

        print(": ");
        print(opt.desc);
        println();
    }
    return 0;
}

} // namespace proto

