#include "components/base_goods_supplier/trader.hpp"

#include "doctest.h"
#include "tests/util.hpp"
#include "values/goods.hpp"
#include "world/base_goods.hpp"

namespace abm::base_goods::supplier {
namespace {
TEST_CASE("Traderのテスト") {  // NOLINT
    constexpr auto id = AgentID{42};

    auto rng    = makeRng();
    auto trader = Trader{rng};
    auto market = Market{};

    SUBCASE("デフォルトで結果は空") {
        const auto result = trader.trade();

        CHECK(result.soldAmount.isZero());
        CHECK(result.unsoldAmount.isZero());
        CHECK(result.totalDemand.isZero());
        CHECK(result.sales.isZero());
    }

    SUBCASE("供給量がゼロの場合、市場にエントリーせず、結果は空") {
        trader.post(
            id, {.price = Price{rng.rand(1.0, 100.0)}, .supply = GoodsQuantity{0.0}}, market
        );

        const auto entry = market.pickEntry(AgentID{-1}, 1, rng);
        CHECK(not entry.has_value());

        const auto result = trader.trade();

        CHECK(result.soldAmount.isZero());
        CHECK(result.unsoldAmount.isZero());
        CHECK(result.totalDemand.isZero());
        CHECK(result.sales.isZero());
    }

    SUBCASE("供給量が正のとき") {
        constexpr auto price  = Price{10};
        constexpr auto supply = GoodsQuantity{100};

        trader.post(id, {.price = price, .supply = supply}, market);

        const auto pick = market.pickEntry(AgentID{-1}, 1, rng);

        CHECK(pick);
        auto& entry = *pick;

        SUBCASE("市場から発見可能で、エントリーの内容が恒等") {
            CHECK(entry.id == id);
            CHECK(entry.price == price);
            CHECK(entry.supply == supply);
        }

        SUBCASE("誰も取引しない場合、需要/売上がゼロ") {
            const auto result = trader.trade();

            CHECK(result.soldAmount.isZero());
            CHECK(result.unsoldAmount.value() == supply.value());
            CHECK(result.totalDemand.isZero());
            CHECK(result.sales.isZero());
        }

        constexpr auto calcPayment = [price](double amount) constexpr noexcept -> Money {
            return price * GoodsQuantity{amount};
        };

        SUBCASE("供給量と同じリクエストを受けた場合、正しい結果を返す") {
            auto& req1 = entry.request(calcPayment(30));
            auto& req2 = entry.request(calcPayment(70));

            const auto result = trader.trade();

            CHECK(result.soldAmount.value() == 100.0);
            CHECK(result.unsoldAmount.value() == 0.0);
            CHECK(result.totalDemand.value() == 100.0);
            CHECK(result.sales.value() == 1000.0);

            CHECK(req1.takeoutTradeAmount().value() == 30.0);
            CHECK(req1.takeoutRemainPaid().value() == 0.0);
            CHECK(req2.takeoutTradeAmount().value() == 70.0);
            CHECK(req2.takeoutRemainPaid().value() == 0.0);
        }

        SUBCASE("供給量より小さいリクエストを受けた場合、正しい結果を返す") {
            auto& req1 = entry.request(calcPayment(40));
            auto& req2 = entry.request(calcPayment(30));

            const auto result = trader.trade();

            CHECK(result.soldAmount.value() == 70.0);
            CHECK(result.unsoldAmount.value() == 30.0);
            CHECK(result.totalDemand.value() == 70.0);
            CHECK(result.sales.value() == 700.0);

            CHECK(req1.takeoutTradeAmount().value() == 40.0);
            CHECK(req1.takeoutRemainPaid().value() == 0.0);
            CHECK(req2.takeoutTradeAmount().value() == 30.0);
            CHECK(req2.takeoutRemainPaid().value() == 0.0);
        }
    }
}
}  // namespace
}  // namespace abm::base_goods::supplier