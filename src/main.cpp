#include <iostream>
#include <map>

#include "allocator.h"
#include "factorial.h"
#include "bidirectional_list.h"

using namespace std;
using namespace griha;

template <typename Comp, typename Alloc>
void fill(map<int, int, Comp, Alloc>& m) {
    for (int i = 0; i < 10; ++i)
        m.emplace(i, fact(i));
}

template <typename Alloc>
void fill(bidirectional_list<int, Alloc>& l) {
    for (int i = 0; i < 10; ++i)
        l.emplace(l.end(), i);
}
namespace std {

template<typename Ch, typename T1, typename T2>
basic_ostream<Ch, char_traits<Ch>>&
operator<<(basic_ostream<Ch, char_traits<Ch>>& os, const std::pair<T1, T2>& p) {
    return os << get<0>(p) << ' ' << get<1>(p);
}

}

int main() {
    map<int, int> map_std_alloc;
    fill(map_std_alloc);

    using alloc_arena_map_t = allocator_arena<pair<const int, int>, 10ul>;
    map<int, int, less<>, alloc_arena_map_t> map_alloc_arena;
    fill(map_alloc_arena);
    copy(begin(map_alloc_arena), end(map_alloc_arena),
            ostream_iterator<pair<const int, int>>(cout, "\n"));
    flush(cout);

    bidirectional_list<int> blist;
    fill(blist);

    using alloc_arena_blist_t = allocator_arena<int, 10ul>;
    bidirectional_list<int, alloc_arena_blist_t> blist_alloc_arena;
    fill(blist_alloc_arena);
    copy(begin(blist_alloc_arena), end(blist_alloc_arena), ostream_iterator<int>(cout, " "));
    endl(cout);

    return 0;
}