#include "components/government.hpp"

#include "doctest.h"
#include "others/util.hpp"

namespace abm::government {
namespace {
TEST_CASE("Governmentのテスト") {  // NOLINT
    auto gov = Government{};

    SUBCASE("デフォルトで資産はゼロ") { CHECK(gov.asset().isZero()); }

    SUBCASE("納税した場合、資産が増加する") {
        SUBCASE("所得税") {
            nothing(gov.payIncomeTax(Money{10}));
            CHECK(gov.asset().isPositive());
        }
        SUBCASE("売上税") {
            nothing(gov.paySalesTax(Money{10}));
            CHECK(gov.asset().isPositive());
        }
        SUBCASE("法人税") {
            nothing(gov.payCorporateTax(Money{10}));
            CHECK(gov.asset().isPositive());
        }
    }
}
}  // namespace
}  // namespace abm::government