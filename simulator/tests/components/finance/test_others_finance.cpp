#include "components/finance/others_finance.hpp"

#include <algorithm>
#include <array>
#include <execution>
#include <functional>

#include "doctest.h"
#include "others/util.hpp"
#include "tests/util.hpp"
#include "values/common.hpp"

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

TEST_CASE("GovernmentFinanceのテスト") {  // NOLINT
    auto rng     = makeRng();
    auto finance = GovernmentFinance{};

    SUBCASE("デフォルトで資産は0") { CHECK(finance.asset().isZero()); }

    SUBCASE("預金した分、資産が増えていること") {
        const auto deposit = rng.rand(1, 1000);
        finance.deposit(Money{deposit});
        CHECK(finance.asset().value() == doctest::Approx(deposit));
    }

    SUBCASE("出金額>資産のとき、資産が戻り値となり、残高が0となる") {
        const auto deposit = 100.0;
        finance.deposit(Money{deposit});

        const auto withdraw = finance.tryWithdraw(Budget{150.0});

        CHECK(withdraw.value() == doctest::Approx(100.0));
        CHECK(finance.asset().value() == doctest::Approx(0.0));
    }

    SUBCASE("出金額==資産のとき、資産が戻り値となり、残高が0となる") {
        const auto deposit = 100.0;
        finance.deposit(Money{deposit});

        const auto withdraw = finance.tryWithdraw(Budget{100.0});

        CHECK(withdraw.value() == doctest::Approx(100.0));
        CHECK(finance.asset().value() == doctest::Approx(0.0));
    }

    SUBCASE("出金額<資産のとき、出金額が戻り値となり、残高がその分引かれる") {
        const auto deposit = 100.0;
        finance.deposit(Money{deposit});

        const auto withdraw = finance.tryWithdraw(Budget{80.0});

        CHECK(withdraw.value() == doctest::Approx(80.0));
        CHECK(finance.asset().value() == doctest::Approx(20.0));
    }

    SUBCASE("並列で資産の追加を行うことが可能") {
        const auto depositArr = std::array<double, 5>{
            rng.rand(10.0, 1000.0),
            rng.rand(10.0, 1000.0),
            rng.rand(10.0, 1000.0),
            rng.rand(10.0, 1000.0),
            rng.rand(10.0, 1000.0)
        };

        const auto sum = std::ranges::fold_left(depositArr, 0.0, std::plus{});

        std::for_each(
            std::execution::par,
            depositArr.begin(),
            depositArr.end(),
            [&](double amount) -> void { finance.deposit(Money{amount}); }
        );

        const auto before = finance.asset();

        CHECK(before.value() == doctest::Approx(sum));
    }

    SUBCASE("並列で資産の引き出しを行うことが可能") {
        const auto withdrawArr = std::array<double, 5>{
            rng.rand(10.0, 100.0),
            rng.rand(10.0, 100.0),
            rng.rand(10.0, 100.0),
            rng.rand(10.0, 100.0),
            rng.rand(10.0, 100.0)
        };

        const auto sum = std::ranges::fold_left(withdrawArr, 0.0, std::plus{});

        finance.deposit(Money{5000});

        std::for_each(
            std::execution::par,
            withdrawArr.begin(),
            withdrawArr.end(),
            [&](double amount) -> void { nothing(finance.tryWithdraw(Budget{amount})); }
        );

        const auto before = finance.asset();

        CHECK(before.value() == doctest::Approx(5000 - sum));
    }
}
}  // namespace
}  // namespace abm