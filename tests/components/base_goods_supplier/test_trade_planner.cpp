#include "components/base_goods_supplier/trade_planner.hpp"

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
}  // namespace
}  // namespace abm::base_goods::supplier