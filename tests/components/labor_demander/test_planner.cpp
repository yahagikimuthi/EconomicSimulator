#include "components/labor_demander/planner.hpp"

#include "components/labor_demander/mediator.hpp"
#include "doctest.h"
#include "tests/util.hpp"
#include "values/common.hpp"
#include "values/labor.hpp"

namespace abm::labor::demander::planner {
TEST_CASE("RecruitPlannerのテスト") {  // NOLINT
    auto rng      = makeRng();
    auto planner  = RecruitPlanner{rng};
    auto mediator = Mediator{};
    planner.acceptMediator(mediator);

    SUBCASE("何もしない場合、1,2回目の結果は等しい") {
        const auto first  = planner.plan(HeadCount{10.0}, Money{1.0});
        const auto second = planner.plan(HeadCount{10.0}, Money{1.0});

        CHECK(first.wage == second.wage);
        CHECK(first.employ == second.employ);
        CHECK(first.offer == second.offer);
    }
}
}  // namespace abm::labor::demander::planner