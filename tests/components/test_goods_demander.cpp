#include "components/goods_demander.hpp"

#include "components/finance/others_finance.hpp"
#include "doctest.h"
#include "tests/util.hpp"

namespace abm::goods::demander {
namespace {
TEST_CASE("Traderのテスト") {  // NOLINT
    auto rng     = makeRng();
    auto trader  = Trader{rng};
    auto finance = HHoldFinance{AgentID{42}, rng};
    auto date    = Date{1};
    auto market  = Market{date};
}
}  // namespace
}  // namespace abm::goods::demander