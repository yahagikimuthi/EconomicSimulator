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
        const auto price  = Price{rng.rand(1.0, 100.0)};
        const auto supply = GoodsQuantity{rng.rand(1.0, 100.0)};

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
    }
}
}  // namespace
}  // namespace abm::base_goods::supplier