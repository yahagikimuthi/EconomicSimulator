#include "components/capital_demander.hpp"

#include <optional>

#include "components/finance/others_finance.hpp"
#include "doctest.h"
#include "others/util.hpp"
#include "tests/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "world/base_goods.hpp"

namespace abm::capital::demander {
class ReadCapital {
  public:
    explicit ReadCapital() noexcept = default;

    void operator()(GoodsQuantity Capital) noexcept { capital = Capital; }

    std::optional<GoodsQuantity> capital;
};

namespace {
TEST_CASE("CapitalDemanderのテスト") {  // NOLINT
    auto rng      = makeRng();
    auto demander = CapitalDemander{rng};
    auto finance  = HHoldFinance{AgentID{42}, rng};
    finance.deposit(Money{2000.0});
    auto market = Market{};

    constexpr auto id = AgentID{42};

    const auto initAsset = finance.asset();

    SUBCASE("デフォルトではリクエストしても資産は減少せず結果は空") {
        const auto budget = demander.planBudget(GoodsQuantity{10.0});
        demander.revisePlan(budget);
        demander.request(id, finance.makeWithdrawFn(), market);
        const auto asset = finance.asset();

        CHECK(asset.value() == initAsset.value());

        auto reader = ReadCapital{};

        demander.afterTrade(finance.makeDepositFn(), reader);

        const auto afterAsset = finance.asset();
        CHECK(afterAsset.value() == asset.value());
        CHECK(not reader.capital);
    }

    SUBCASE("同じIDのみが市場にポストされている場合も同様に空") {
        auto& entry = market.entry(id, Price{10}, GoodsQuantity{10});

        const auto budget = demander.planBudget(GoodsQuantity{10.0});
        demander.revisePlan(budget);
        demander.request(id, finance.makeWithdrawFn(), market);
        const auto asset = finance.asset();

        CHECK(asset.value() == initAsset.value());
        CHECK(entry.requests().empty());

        auto reader = ReadCapital{};

        demander.afterTrade(finance.makeDepositFn(), reader);

        const auto afterAsset = finance.asset();
        CHECK(afterAsset.value() == asset.value());
        CHECK(not reader.capital);
    }

    SUBCASE("購入計画*価格>予算のとき、予算分を支払う") {
        nothing(demander.planBudget(GoodsQuantity{10.0}));
        demander.revisePlan(Budget{100.0});

        auto& entry = market.entry(AgentID{101}, Price{20}, GoodsQuantity{10});

        demander.request(id, finance.makeWithdrawFn(), market);

        CHECK(initAsset.value() - finance.asset().value() == doctest::Approx(100.0));
        CHECK(entry.requests().size() == 1UZ);
        CHECK(entry.requests().front().payment.value() == 100.0);
    }

    SUBCASE("購入計画*価格==予算のとき、予算分を支払う") {
        nothing(demander.planBudget(GoodsQuantity{10.0}));
        demander.revisePlan(Budget{100.0});

        auto& entry = market.entry(AgentID{101}, Price{10}, GoodsQuantity{10});

        demander.request(id, finance.makeWithdrawFn(), market);

        CHECK(initAsset.value() - finance.asset().value() == doctest::Approx(100.0));
        CHECK(entry.requests().size() == 1UZ);
        CHECK(entry.requests().front().payment.value() == 100.0);
    }

    SUBCASE("購入計画*価格<予算のとき、計画分を支払う") {
        nothing(demander.planBudget(GoodsQuantity{10.0}));
        demander.revisePlan(Budget{150.0});

        auto& entry = market.entry(AgentID{101}, Price{5}, GoodsQuantity{10});

        demander.request(id, finance.makeWithdrawFn(), market);

        CHECK(initAsset.value() - finance.asset().value() == doctest::Approx(50.0));
        CHECK(entry.requests().size() == 1UZ);
        CHECK(entry.requests().front().payment.value() == 50.0);
    }

    SUBCASE("リクエスト済みのとき") {
        const auto reqBudget = demander.planBudget(GoodsQuantity{10.0});
        demander.revisePlan(Budget{100.0});
        auto& entry = market.entry(AgentID{101}, Price{10}, GoodsQuantity{10.0});
        demander.request(id, finance.makeWithdrawFn(), market);
        const auto             asset   = finance.asset();
        [[maybe_unused]] auto& request = entry.requests().front();

        SUBCASE("取引を行わない場合、払い戻しを行い、購入生産財数はゼロ、ログを更新しない") {
            auto reader = ReadCapital{};
            demander.afterTrade(finance.makeDepositFn(), reader);

            CHECK(finance.asset().value() == asset.value() + 100.0);
            CHECK(reader.capital);
            CHECK(reader.capital->isZero());

            const auto afterReqBudget = demander.planBudget(GoodsQuantity{10.0});

            CHECK(afterReqBudget.value() == reqBudget.value());
        }

        SUBCASE("取引を一部行う場合、払い戻しを行い、購入生産財数が存在、ログを更新する") {
            auto       reader = ReadCapital{};
            const auto pay    = request.trade(GoodsQuantity{7});
            demander.afterTrade(finance.makeDepositFn(), reader);

            CHECK(finance.asset().value() == doctest::Approx(initAsset.value() - pay.value()));
            CHECK(reader.capital);
            CHECK(reader.capital->value() == 7);

            const auto afterReqBudget = demander.planBudget(GoodsQuantity{10.0});

            CHECK(afterReqBudget.value() != reqBudget.value());
        }

        SUBCASE("取引をすべて行う場合、払い戻しをせず、購入生産財数が存在、ログを更新する") {
            auto       reader = ReadCapital{};
            const auto pay    = request.trade(GoodsQuantity{10});
            demander.afterTrade(finance.makeDepositFn(), reader);

            CHECK(pay.value() == 100.0);

            CHECK(finance.asset().value() == doctest::Approx(initAsset.value() - pay.value()));
            CHECK(reader.capital);
            CHECK(reader.capital->value() == 10);

            const auto afterReqBudget = demander.planBudget(GoodsQuantity{10.0});

            CHECK(afterReqBudget.value() != reqBudget.value());
        }

        SUBCASE("2回目のafterTradeの結果は空") {
            auto reader = ReadCapital{};
            nothing(request.trade(GoodsQuantity{7}));
            demander.afterTrade(finance.makeDepositFn(), reader);

            const auto beforeAsset = finance.asset();

            auto reader2 = ReadCapital{};

            demander.afterTrade(finance.makeDepositFn(), reader2);

            CHECK(finance.asset().value() == beforeAsset.value());
            CHECK(not reader2.capital);
        }
    }
}
}  // namespace
}  // namespace abm::capital::demander