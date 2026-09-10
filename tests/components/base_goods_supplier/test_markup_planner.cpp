#include "components/base_goods_supplier/markup_planner.hpp"

#include "components/base_goods_supplier/mediator.hpp"
#include "doctest.h"
#include "tests/util.hpp"
#include "values/goods.hpp"

namespace abm::base_goods::supplier {
TEST_CASE("MarkupPlannerMemoryのテスト") {  // NOLINT
    auto rng      = makeRng();
    auto mediator = Mediator{};
    auto memory   = MarkupPlannerMemory{rng};
    mediator.subscribeTradePlan(memory);
    mediator.subscribeTradeResult(memory);

    SUBCASE("供給計画がゼロの場合、更新処理を行わない") {
        const auto beforeLastSupply      = memory.lastSupply().value();
        const auto beforeLastSalesAmount = memory.lastSalesAmount().value();

        mediator.publishTradePlan({.price = Price{10.0}, .supply = GoodsQuantity{0.0}});
        mediator.publishTradeResult(
            {.soldAmount   = GoodsQuantity{10.0},
             .unsoldAmount = GoodsQuantity{1.0},
             .totalDemand  = GoodsQuantity{100.0},
             .sales        = Money{10.0}}
        );

        const auto afterLastSupply      = memory.lastSupply().value();
        const auto afterLastSalesAmount = memory.lastSalesAmount().value();

        CHECK(beforeLastSupply.value() == afterLastSupply.value());
        CHECK(beforeLastSalesAmount.value() == afterLastSalesAmount.value());
    }

    SUBCASE("供給計画がゼロでない場合、更新処理を行う") {
        mediator.publishTradePlan({.price = Price{10.0}, .supply = GoodsQuantity{100.0}});
        mediator.publishTradeResult(
            {.soldAmount   = GoodsQuantity{10.0},
             .unsoldAmount = GoodsQuantity{1.0},
             .totalDemand  = GoodsQuantity{100.0},
             .sales        = Money{10.0}}
        );

        const auto lastSupply      = memory.lastSupply().value();
        const auto lastSalesAmount = memory.lastSalesAmount().value();

        CHECK(lastSupply.value() == 100.0);
        CHECK(lastSalesAmount.value() == 10.0);
    }
}
}  // namespace abm::base_goods::supplier