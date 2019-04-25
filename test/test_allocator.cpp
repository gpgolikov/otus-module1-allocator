#include <catch2/catch.hpp>

#include <allocator.h>

#include "utils.h"

using namespace std;
using namespace griha;
using namespace Catch::Matchers;

TEST_CASE("allocation") {
    allocator_arena<int, 10ul> alloc;
    SECTION("allocation in same chunk") {
        auto p1 = alloc.allocate(6ul);
        auto p2 = alloc.allocate(4ul);
        REQUIRE(&p1[6ul] == p2); // p2 should be next after end of p1
    }

    SECTION("allocation with extend to one more chunk") {
        auto p1 = alloc.allocate(6ul);
        auto p2 = alloc.allocate(5ul);
        for (auto i = 0ul; i < 10ul; ++i)
            REQUIRE_FALSE(&p1[i] == p2); // p2 should be allocate in another chunk then p1
        auto p3 = alloc.allocate(5ul);
        REQUIRE(&p2[5ul] == p3); // p3 should be next after end of p2
    }
}

TEST_CASE("deallocation") {
    allocator_arena<int, 10ul> alloc;
    SECTION("deallocation frees space") {
        auto p1 = alloc.allocate(6ul);
        auto p2 = alloc.allocate(3ul);
        alloc.deallocate(p2, 3ul); // n should be same as in corresponding call of allocate
        auto p3 = alloc.allocate(4ul);
        REQUIRE(&p1[6ul] == p3);
        REQUIRE(p2 == p3); // allocation perfoms in freed space
    }

    SECTION("fragmentation") {
        alloc.allocate(6ul);
        auto p1 = alloc.allocate(2ul);
        alloc.allocate(2ul);
        alloc.deallocate(p1, 2ul);
        auto p2 = alloc.allocate(1ul);
        auto p3 = alloc.allocate(1ul);
        REQUIRE(p1 == p2); // allocation perfoms in freed space
        REQUIRE(&p1[1ul] == p3); // allocation perfoms in freed fragmentated space
    }

    SECTION("fragmentation with extend to one more chunk") {
        alloc.allocate(6ul);
        auto p1 = alloc.allocate(2ul);
        alloc.allocate(2ul);
        alloc.deallocate(p1, 2ul);
        auto p2 = alloc.allocate(1ul);
        auto p3 = alloc.allocate(2ul);
        REQUIRE(p1 == p2); // allocation perfoms in freed space
        REQUIRE_FALSE(&p1[1ul] == p3); // allocation perfoms in freed fragmentated space

        alloc.allocate(8ul); // new chunk is first in chunk list
        auto p4 = alloc.allocate(1ul); // now it allocates in old chunk
        REQUIRE(&p2[1ul] == p4); // p4 should be next after end of p2
    }
}

TEST_CASE("construction") {
    using Struct = std::pair<int, int>;
    allocator_arena<Struct, 5> alloc;
    SECTION("construct two sequential objects") {
        auto p1 = alloc.allocate(1);
        auto p2 = alloc.allocate(1);

        alloc.construct(p1, 1, 2);
        alloc.construct(p2, 3, 4);

        REQUIRE_THAT(*p1, Equals(std::make_pair(1, 2)));
        REQUIRE_THAT(*p2, Equals(std::make_pair(3, 4)));
    }
}

struct Struct {
    int field;
    explicit Struct(int f) : field(f) {}
    ~Struct() { field = 0; }
};

inline bool operator== (const Struct& lhs, const Struct& rhs) {
    return lhs.field == rhs.field;
}

template<typename Ch>
std::basic_ostream<Ch, std::char_traits<Ch>>&
operator<<(std::basic_ostream<Ch, std::char_traits<Ch>>& os, const Struct& s) {
    return os << '{' << s.field << '}';
}

TEST_CASE("destruction") {
    allocator_arena<Struct, 5> alloc;
    SECTION("destruct object") {
        auto p1 = alloc.allocate(1);

        alloc.construct(p1, 1);
        alloc.destroy(p1);
        REQUIRE_THAT(*p1, Equals(Struct(0)));
    }

    SECTION("destruct in middle") {
        auto p1 = alloc.allocate(1);
        auto p2 = alloc.allocate(2);

        alloc.construct(p1, 1);
        alloc.construct(p2, 2);
        alloc.construct(&p2[1], 3);
        REQUIRE_THAT(*p1, Equals(Struct(1)));
        REQUIRE_THAT(p2[0], Equals(Struct(2)));
        REQUIRE_THAT(p2[1], Equals(Struct(3)));

        alloc.destroy(p2);
        REQUIRE_THAT(*p1, Equals(Struct(1)));
        REQUIRE_THAT(p2[0], Equals(Struct(0)));
        REQUIRE_THAT(p2[1], Equals(Struct(3)));
    }
}