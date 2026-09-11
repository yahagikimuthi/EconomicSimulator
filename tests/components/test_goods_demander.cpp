#include "components/goods_demander.hpp"

#include "components/finance/others_finance.hpp"
#include "doctest.h"
#include "tests/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "world/base_goods.hpp"

namespace abm::goods::demander {
namespace {
TEST_CASE("Traderのテスト") {  // NOLINT
    auto rng     = makeRng();
    auto trader  = Trader{rng};
    auto finance = HHoldFinance{AgentID{42}, rng};
    auto market  = Market{};

    finance.deposit(Money{2000.0});

    const auto beforeAsset = finance.asset();

    SUBCASE("何もポストされていない場合、リクエストしても資産が減らない") {
        trader.request(AgentID{42}, Budget{100}, finance.makeWithdrawFn(), market, 1);

        const auto afterAsset = finance.asset();

        CHECK(afterAsset.value() == beforeAsset.value());
    }

    SUBCASE("同じIDでポストされていたとき、資産が減らず、リクエストされない") {
        auto& entry = market.entry(AgentID{42}, Price{10}, GoodsQuantity{100.0});
        trader.request(AgentID{42}, Budget{100}, finance.makeWithdrawFn(), market, 1);

        const auto afterAsset = finance.asset();

        CHECK(afterAsset.value() == beforeAsset.value());
        CHECK(entry.requests().empty());
    }

    SUBCASE("探索回数が十分に大きい場合、IDが異なり最も安価なものにエントリーする") {
        constexpr auto supply = GoodsQuantity{100.0};
        constexpr auto id     = AgentID{42};

        auto& e1 = market.entry(id, Price{12}, supply);
        auto& e2 = market.entry(id, Price{5}, supply);
        auto& e3 = market.entry(AgentID{101}, Price{35}, supply);
        auto& e4 = market.entry(AgentID{202}, Price{10}, supply);
        auto& e5 = market.entry(AgentID{303}, Price{100}, supply);

        trader.request(id, Budget{rng.rand(1, 1000)}, finance.makeWithdrawFn(), market, 100);

        CHECK(e1.requests().empty());
        CHECK(e2.requests().empty());
        CHECK(e3.requests().empty());
        CHECK(e4.requests().size() == 1UZ);
        CHECK(e5.requests().empty());
    }

    SUBCASE("1つだけ対象のエントリーが存在する場合、資産が減りリクエストしていること") {
        auto& entry = market.entry(AgentID{101}, Price{10}, GoodsQuantity{100.0});
        trader.request(AgentID{42}, Budget{100}, finance.makeWithdrawFn(), market, 1);

        const auto afterAsset = finance.asset();
        CHECK(afterAsset.value() < beforeAsset.value());

        CHECK(entry.requests().size() == 1UZ);

        auto& req = entry.requests().front();
        CHECK(req.payment.value() == 100);

        SUBCASE("支払い残高がゼロの場合、取引後処理において資産が増加しない") {
            const auto sales = req.trade(GoodsQuantity{10});

            CHECK(sales.value() == 100.0);

            trader.afterTrade(finance.makeDepositFn());

            const auto finalAsset = finance.asset();

            CHECK(finalAsset.value() == afterAsset.value());
        }

        SUBCASE("支払い残高が存在する場合、取引後処理において資産が増加する") {
            const auto sales = req.trade(GoodsQuantity{1});

            CHECK(sales.value() == 10.0);

            trader.afterTrade(finance.makeDepositFn());

            const auto finalAsset = finance.asset();

            CHECK(finalAsset.value() - afterAsset.value() == 90);
        }

        SUBCASE("取引後、再度、afterTradeを呼び出しても資産が変わらないこと") {
            const auto sales = req.trade(GoodsQuantity{1});
            CHECK(sales.value() == 10.0);

            trader.afterTrade(finance.makeDepositFn());
            const auto finalAsset = finance.asset();

            trader.afterTrade(finance.makeDepositFn());
            const auto secondBeforeAsset = finance.asset();

            CHECK(finalAsset.value() == secondBeforeAsset.value());
        }
    }
}
}  // namespace
}  // namespace abm::goods::demander