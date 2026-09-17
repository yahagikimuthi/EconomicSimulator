#include "components/base_goods_supplier/markup_planner.hpp"

#include "components/base_goods_supplier/mediator.hpp"
#include "doctest.h"
#include "tests/util.hpp"
#include "values/goods.hpp"

namespace abm::base_goods::supplier {
namespace {
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

TEST_CASE("MarkupPlannerのテスト") {  // NOLINT
    auto rng      = makeRng();
    auto mediator = Mediator{};
    auto planner  = MarkupPlanner{rng};
    planner.acceptMediator(mediator);
    const auto targetInvRatio = 0.1;

    SUBCASE("mediateしない場合、1回目と2回目の結果が同じであること") {
        const auto before = planner.plan(targetInvRatio);
        const auto after  = planner.plan(targetInvRatio);

        CHECK(before.value() == after.value());
    }

    SUBCASE("在庫/供給が目標在庫率と等しい場合、マークアップ率を上げる") {
        const auto before = planner.plan(targetInvRatio);

        mediator.publishTradePlan({.price = Price{10.0}, .supply = GoodsQuantity{100.0}});
        mediator.publishTradeResult(
            {.soldAmount   = GoodsQuantity{90},
             .unsoldAmount = GoodsQuantity{10},
             .totalDemand  = GoodsQuantity{90},
             .sales        = Money{900}}
        );

        const auto after = planner.plan(targetInvRatio);

        CHECK(after.value() > before.value());
    }

    SUBCASE("在庫/供給が目標在庫率より大きい場合、マークアップ率を下げる") {
        const auto before = planner.plan(targetInvRatio);

        mediator.publishTradePlan({.price = Price{10.0}, .supply = GoodsQuantity{100.0}});
        mediator.publishTradeResult(
            {.soldAmount   = GoodsQuantity{80},
             .unsoldAmount = GoodsQuantity{20},
             .totalDemand  = GoodsQuantity{80},
             .sales        = Money{800}}
        );

        const auto after = planner.plan(targetInvRatio);

        CHECK(after.value() < before.value());
    }

    SUBCASE("在庫/供給が目標在庫率より小さい場合、マークアップ率を上げる") {
        const auto before = planner.plan(targetInvRatio);

        mediator.publishTradePlan({.price = Price{10.0}, .supply = GoodsQuantity{100.0}});
        mediator.publishTradeResult(
            {.soldAmount   = GoodsQuantity{90},
             .unsoldAmount = GoodsQuantity{10},
             .totalDemand  = GoodsQuantity{90},
             .sales        = Money{900}}
        );

        const auto after = planner.plan(targetInvRatio);

        CHECK(after.value() > before.value());
    }
}
}  // namespace
}  // namespace abm::base_goods::supplier