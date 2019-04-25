#include <catch2/catch.hpp>

#include <factorial.h>

#include "utils.h"

using namespace std;
using namespace griha;
using namespace Catch::Matchers;

TEST_CASE("factorial") {
    SECTION("template") {
        REQUIRE_THAT(factorial_v<0ul>, Equals(1ul));
        REQUIRE_THAT(factorial_v<1ul>, Equals(1ul));
        REQUIRE_THAT(factorial_v<2ul>, Equals(2ul));
        REQUIRE_THAT(factorial_v<3ul>, Equals(6ul));
        REQUIRE_THAT(factorial_v<4ul>, Equals(24ul));
        REQUIRE_THAT(factorial_v<10ul>, Equals(3628800ul));
    }

    SECTION("function") {
        REQUIRE_THAT(fact(0ul), Equals(1ul));
        REQUIRE_THAT(fact(1ul), Equals(1ul));
        REQUIRE_THAT(fact(2ul), Equals(2ul));
        REQUIRE_THAT(fact(3ul), Equals(6ul));
        REQUIRE_THAT(fact(4ul), Equals(24ul));
        REQUIRE_THAT(fact(10ul), Equals(3628800ul));
    }
}