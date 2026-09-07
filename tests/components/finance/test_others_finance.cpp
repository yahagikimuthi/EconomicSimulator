#include "components/finance/others_finance.hpp"

#include "doctest.h"
#include "tests/util.hpp"

namespace abm {
namespace {
TEST_CASE("HHoldFinanceのテスト") {  // NOLINT
    auto rng     = makeRng();
    auto finance = HHoldFinance{AgentID{42}, rng};

    SUBCASE("乱数生成による構築の場合、資産が正であること") { CHECK(finance.asset().isPositive()); }

    SUBCASE("預金した分、資産が増えていること") {
        constexpr auto deposit     = Money{100.0};
        const auto     beforeAsset = finance.asset();

        auto depositFn = finance.makeDepositFn();
        depositFn(deposit);

        CHECK(finance.asset().value() == doctest::Approx(beforeAsset.value() + deposit.value()));
    }

    SUBCASE("出金した分、資産が減っていること") {
        const auto beforeAsset = finance.asset();

        const auto result = finance.makeWithdrawFn()(Budget{50.0});

        CHECK(finance.asset().value() == doctest::Approx(beforeAsset.value() - result.value()));
    }
}
}  // namespace
}  // namespace abm