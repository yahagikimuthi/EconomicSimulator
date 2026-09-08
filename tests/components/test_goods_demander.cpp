#include "components/goods_demander.hpp"

#include "components/finance/others_finance.hpp"
#include "doctest.h"
#include "tests/util.hpp"

namespace abm::goods::demander {
namespace {
TEST_CASE("Traderのテスト") {  // NOLINT
    [[maybe_unused]] auto rng     = makeRng();
    [[maybe_unused]] auto trader  = Trader{rng};
    [[maybe_unused]] auto finance = HHoldFinance{AgentID{42}, rng};
    [[maybe_unused]] auto date    = Date{1};
    [[maybe_unused]] auto market  = Market{date};
}
}  // namespace
}  // namespace abm::goods::demander