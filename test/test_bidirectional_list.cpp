#include <catch2/catch.hpp>

#include <array>
#include <numeric>

#include <bidirectional_list.h>

#include "utils.h"

using namespace std;
using namespace griha;
using namespace Catch::Matchers;

TEST_CASE("bidirectional_list") {
    SECTION("construction") {
        bidirectional_list<pair<int, float>> blist;
        REQUIRE(blist.empty());
        REQUIRE_THAT(blist.size(), Equals(0ul));
        REQUIRE(blist.begin() == blist.end());
        REQUIRE(blist.cbegin() == blist.cend());
    }

    SECTION("emplace") {
        bidirectional_list<pair<int, float>> blist;
        REQUIRE(blist.emplace(blist.end(), 1, 1.) == blist.begin());
        REQUIRE_FALSE(blist.empty());
        REQUIRE_THAT(blist.size(), Equals(1ul));

        REQUIRE_FALSE(blist.begin() == blist.end());
        REQUIRE_FALSE(blist.cbegin() == blist.cend());

        blist.emplace(blist.end(), 2, 2.);
        REQUIRE_THAT(blist.size(), Equals(2ul));

        REQUIRE(blist.emplace(blist.begin(), 3, 3.) == blist.begin());
        REQUIRE_THAT(blist.size(), Equals(3ul));

        auto it = blist.begin();
        ++it;
        blist.emplace(it, 4, 4.);
        REQUIRE_THAT(blist.size(), Equals(4ul));

        it = blist.begin();
        REQUIRE_THAT(*it, Equals(make_pair(3, 3.)));
        ++it;
        REQUIRE_THAT(*it, Equals(make_pair(4, 4.)));
        ++it;
        REQUIRE_THAT(*it, Equals(make_pair(1, 1.)));
        ++it;
        REQUIRE_THAT(*it, Equals(make_pair(2, 2.)));
        ++it;
        REQUIRE(it == blist.end());
    }

    SECTION("erase") {
        bidirectional_list<pair<int, float>> blist;
        blist.emplace(blist.end(), 1, 1.);
        blist.emplace(blist.end(), 2, 2.);
        blist.emplace(blist.end(), 3, 3.);
        blist.emplace(blist.end(), 4, 4.);
        REQUIRE_THAT(blist.size(), Equals(4ul));

        auto it = blist.begin();
        ++it;
        blist.erase(it);
        REQUIRE_FALSE(blist.empty());
        REQUIRE_THAT(blist.size(), Equals(3ul));
        it = blist.begin();
        REQUIRE_THAT(*it, Equals(make_pair(1, 1.)));
        ++it;
        REQUIRE_THAT(*it, Equals(make_pair(3, 3.)));
        ++it;
        REQUIRE_THAT(*it, Equals(make_pair(4, 4.)));
        ++it;
        REQUIRE(it == blist.end());

        it = blist.end();
        --it;
        blist.erase(it);
        REQUIRE_FALSE(blist.empty());
        REQUIRE_THAT(blist.size(), Equals(2ul));
        it = blist.begin();
        REQUIRE_THAT(*it, Equals(make_pair(1, 1.)));
        ++it;
        REQUIRE_THAT(*it, Equals(make_pair(3, 3.)));
        ++it;
        REQUIRE(it == blist.end());

        blist.erase(blist.begin());
        REQUIRE_FALSE(blist.empty());
        REQUIRE_THAT(blist.size(), Equals(1ul));
        it = blist.begin();
        REQUIRE_THAT(*it, Equals(make_pair(3, 3.)));
        ++it;
        REQUIRE(it == blist.end());

        blist.erase(blist.begin());
        REQUIRE(blist.empty());
        REQUIRE_THAT(blist.size(), Equals(0ul));
        REQUIRE(blist.begin() == blist.end());
    }

    SECTION("range-for") {
        bidirectional_list<int> blist;
        std::array<int, 10> values;
        iota(begin(values), end(values), 1);

        for (auto v : values)
            blist.emplace(end(blist), v);

        auto it = begin(values);
        for (auto v : blist) {
            REQUIRE_THAT(v, Equals(*it));
            ++it;
        }
    }

    SECTION("copy") {
        bidirectional_list<int> blist;
        std::array<int, 10> values;
        iota(begin(values), end(values), 1);

        bidirectional_list<int> blist_copy(blist);
        auto it = begin(values);
        for (auto v : blist_copy) {
            REQUIRE_THAT(v, Equals(*it));
            ++it;
        }
    }

    SECTION("assignmnet") {
        bidirectional_list<int> blist;
        std::array<int, 10> values;
        iota(begin(values), end(values), 1);

        bidirectional_list<int> blist_copy;
        blist_copy = blist;
        auto it = begin(values);
        for (auto v : blist_copy) {
            REQUIRE_THAT(v, Equals(*it));
            ++it;
        }
    }
    
    SECTION("move") {
        bidirectional_list<int> blist;
        std::array<int, 10> values;
        iota(begin(values), end(values), 1);

        bidirectional_list<int> blist_move(move(blist));
        auto it = begin(values);
        for (auto v : blist_move) {
            REQUIRE_THAT(v, Equals(*it));
            ++it;
        }
        REQUIRE(blist.empty());
    }

    SECTION("move-assignmnet") {
        bidirectional_list<int> blist;
        std::array<int, 10> values;
        iota(begin(values), end(values), 1);

        bidirectional_list<int> blist_move;
        blist_move.emplace(end(blist_move), 100);

        blist_move = move(blist);
        auto it = begin(values);
        for (auto v : blist_move) {
            REQUIRE_THAT(v, Equals(*it));
            ++it;
        }
        REQUIRE_THAT(blist.size(), Equals(1ul));
        REQUIRE_THAT(*blist.begin(), Equals(100));
    }
}