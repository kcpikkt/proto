#pragma once

namespace proto {
template<typename Fst, typename Snd> 
struct Pair {
    Fst first;
    Snd second;
};

// should RVO
//template<typename Fst, typename Snd>
//inline Pair<Fst, Snd> make_pair(Fst& first, Snd& second) {
//    return Pair<Fst, Snd>(first,second);
//}

} // namespace proto
