#include "components/base_goods_supplier/trade_planner.hpp"

#include "components/base_goods_supplier/mediator.hpp"
#include "doctest.h"
#include "tests/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"

namespace abm::base_goods::supplier {
namespace {
TEST_CASE("PricePlannerのテスト（乱数の関係で1%未満だが失敗する可能性がある）") {  // NOLINT
    auto rng     = makeRng();
    auto planner = PricePlanner{rng, 1e-6};

    const auto baseSupply = GoodsQuantity{rng.rand(100.0, 1000.0)};
    const auto baseMarkup = MarkupRate{0.5};
    const auto baseCost   = Budget{rng.rand(100.0, 1000.0)};

    const auto baseOut = planner.plan(baseSupply, baseMarkup, baseCost);

    SUBCASE("供給量が増えた場合、つまり生産性が上がった場合、価格は下がる") {
        const auto out = planner.plan(baseSupply * 2, baseMarkup, baseCost);

        CHECK(out.value() <= baseOut.value());
    }

    SUBCASE("供給量が減った場合、価格が上がる") {
        const auto out = planner.plan(baseSupply / 2, baseMarkup, baseCost);

        CHECK(out.value() >= baseOut.value());
    }

    SUBCASE("マークアップ率が上がった場合、価格は上がる") {
        const auto out = planner.plan(baseSupply, baseMarkup * 1.5, baseCost);

        CHECK(out.value() >= baseOut.value());
    }

    SUBCASE("マークアップ率が下がった場合、価格は下がる") {
        const auto out = planner.plan(baseSupply, baseMarkup / 1.5, baseCost);

        CHECK(out.value() <= baseOut.value());
    }

    SUBCASE("費用が上がった場合、つまり労働生産性が下がった場合、価格は上がる") {
        const auto out = planner.plan(baseSupply, baseMarkup, baseCost * 2);

        CHECK(out.value() >= baseOut.value());
    }

    SUBCASE("費用が下がった場合、価格は下がる") {
        const auto out = planner.plan(baseSupply, baseMarkup, baseCost / 2);

        CHECK(out.value() <= baseOut.value());
    }

    SUBCASE("供給量がゼロの場合、価格は0からイプシロンの間") {
        const auto out = planner.plan(GoodsQuantity{0.0}, baseMarkup, baseCost);

        CHECK(out.isPositive());
        CHECK(out.value() <= 1e-6);
    }
}

TEST_CASE("DemandForecastManagerMemoryのテスト") {  // NOLINT
    auto rng      = makeRng();
    auto mediator = Mediator{};
    auto memory   = DemandForecastManagerMemory{rng};
    mediator.subscribeTradeResult(memory);

    SUBCASE("供給量がゼロの場合、更新処理が行われない") {
        const auto before = memory.lastTotalDemand().value();

        mediator.publishTradeResult(
            {.soldAmount   = GoodsQuantity{0.0},
             .unsoldAmount = GoodsQuantity{0.0},
             .totalDemand  = GoodsQuantity{10.0},
             .sales        = Money{100.0}}
        );

        const auto after = memory.lastTotalDemand().value();

        CHECK(before.value() == after.value());
    }

    SUBCASE("供給量が正の場合、更新される") {
        mediator.publishTradeResult(
            {.soldAmount   = GoodsQuantity{10.0},
             .unsoldAmount = GoodsQuantity{10.0},
             .totalDemand  = GoodsQuantity{35.0},
             .sales        = Money{100.0}}
        );

        const auto demand = memory.lastTotalDemand().value();

        CHECK(demand.value() == 35.0);
    }
}

TEST_CASE("DemandForecastManagerのテスト") {  // NOLINT
    auto rng      = makeRng();
    auto mediator = Mediator{};
    auto manager  = DemandForecastManager{rng};
    manager.acceptMediator(mediator);

    SUBCASE("供給量がゼロのとき、更新処理が行われない") {
        const auto before = manager.plan();

        mediator.publishTradeResult(
            {.soldAmount   = GoodsQuantity{0.0},
             .unsoldAmount = GoodsQuantity{0.0},
             .totalDemand  = GoodsQuantity{350.0},
             .sales        = Money{10.0}}
        );

        const auto after = manager.plan();

        CHECK(before.value() == after.value());
    }

    SUBCASE("何もしない場合、結果は同じ") {
        const auto before = manager.plan();
        const auto after  = manager.plan();

        CHECK(before.value() == after.value());
    }

    SUBCASE("需要量が予想と変わらない場合、結果は同じ") {
        const auto before = manager.plan();

        mediator.publishTradeResult(
            {.soldAmount   = GoodsQuantity{rng.rand(1.0, 100.0)},
             .unsoldAmount = GoodsQuantity{rng.rand(1.0, 100.0)},
             .totalDemand  = before,
             .sales        = Money{rng.rand(1.0, 100.0)}}
        );

        const auto after = manager.plan();

        CHECK(before.value() == after.value());
    }

    SUBCASE("需要量が予想より大きい場合、予測が上方修正") {
        const auto before = manager.plan();

        mediator.publishTradeResult(
            {.soldAmount   = GoodsQuantity{rng.rand(1.0, 100.0)},
             .unsoldAmount = GoodsQuantity{rng.rand(1.0, 100.0)},
             .totalDemand  = before * 2,
             .sales        = Money{rng.rand(1.0, 100.0)}}
        );

        const auto after = manager.plan();

        CHECK(after.value() > before.value());
    }

    SUBCASE("需要量が予想未満の場合、予測が下方修正") {
        const auto before = manager.plan();

        mediator.publishTradeResult(
            {.soldAmount   = GoodsQuantity{rng.rand(1.0, 100.0)},
             .unsoldAmount = GoodsQuantity{rng.rand(1.0, 100.0)},
             .totalDemand  = before / 2,
             .sales        = Money{rng.rand(1.0, 100.0)}}
        );

        const auto after = manager.plan();

        CHECK(after.value() < before.value());
    }

    SUBCASE("需要量がゼロの場合、予測が下方修正") {
        const auto before = manager.plan();

        mediator.publishTradeResult(
            {.soldAmount   = GoodsQuantity{rng.rand(1.0, 100.0)},
             .unsoldAmount = GoodsQuantity{rng.rand(1.0, 100.0)},
             .totalDemand  = GoodsQuantity{0.0},
             .sales        = Money{rng.rand(1.0, 100.0)}}
        );

        const auto after = manager.plan();

        CHECK(after.value() < before.value());
    }
}
}  // namespace
}  // namespace abm::base_goods::supplier