#include "components/base_goods_supplier/produsing.hpp"

#include "doctest.h"
#include "tests/util.hpp"
#include "values/goods.hpp"

namespace abm::base_goods::supplier::producing {
namespace {
TEST_CASE("CapitalManagerのテスト") {  // NOLINT
    auto rng     = makeRng();
    auto manager = CapitalManager{rng};

    SUBCASE("デフォルトで生産量及び計画量はゼロ") {
        CHECK(manager.produce().isZero());
        CHECK(manager.nextProducePlan().isZero());
    }

    SUBCASE("produceとnextProducePlanは一致する") {
        manager.addCapital(GoodsQuantity{rng.rand(1.0, 1000.0)});

        const auto plan    = manager.nextProducePlan();
        const auto produce = manager.produce();
        CHECK(plan.value() == produce.value());
    }

    SUBCASE("生産財を加えたとき、生産量が正、要求生産財が減少") {
        const auto beforeReq = manager.desiredCapital(GoodsQuantity{100.0});
        manager.addCapital(GoodsQuantity{100.0});

        CHECK(manager.nextProducePlan().value() > 0.0);
        CHECK(manager.desiredCapital(GoodsQuantity{100}).value() < beforeReq.value());
    }

    SUBCASE("生産財をさらに加えたとき、生産量がさらに増加、要求生産財がさらに減少") {
        manager.addCapital(GoodsQuantity{100.0});

        const auto beforeReq  = manager.desiredCapital(GoodsQuantity{1000.0});
        const auto beforePlan = manager.nextProducePlan();

        manager.addCapital(GoodsQuantity{200.0});

        const auto afterReq  = manager.desiredCapital(GoodsQuantity{1000.0});
        const auto afterPlan = manager.nextProducePlan();

        CHECK(afterReq.value() < beforeReq.value());
        CHECK(afterPlan.value() > beforePlan.value());
    }

    SUBCASE("2回目の生産量は1回目未満である") {
        manager.addCapital(GoodsQuantity{100.0});

        const auto before = manager.produce();
        const auto after  = manager.produce();

        CHECK(after.value() < before.value());
    }

    SUBCASE("nextProducePlanはconstである") {
        manager.addCapital(GoodsQuantity{100.0});

        const auto before = manager.nextProducePlan();
        const auto after  = manager.nextProducePlan();

        CHECK(before.value() == after.value());
    }
}
}  // namespace
}  // namespace abm::base_goods::supplier::producing