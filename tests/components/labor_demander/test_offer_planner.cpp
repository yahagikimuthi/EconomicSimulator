#include "components/labor_demander/common.hpp"
#include "components/labor_demander/offer_planner.hpp"

#include "doctest.h"
#include "tests/util.hpp"
#include "values/labor.hpp"

namespace abm::labor::demander::planner {
TEST_CASE("OfferPlannerMemoryのテスト") {  // NOLINT
    auto rng    = makeRng();
    auto memory = OfferPlannerMemory{rng};

    SUBCASE("雇用計画が0のとき、雇用結果を更新しないこと") {
        const auto plan =
            RecruitPlan{.wage = Wage{1.0}, .employ = HeadCount{0.0}, .offer = HeadCount{0.0}};
        const auto result = RecruitResult{.applicants = HeadCount{100.0}, .employ = HeadCount{0.0}};
        const auto lastPlan   = *memory.lastEmployPlan();
        const auto lastResult = *memory.lastEmployResult();

        memory.listenRecruitPlan(plan);
        memory.listenRecruitResult(result);

        CHECK(memory.lastEmployPlan() == lastPlan);
        CHECK(memory.lastEmployResult() == lastResult);
    }
}
}  // namespace abm::labor::demander::planner