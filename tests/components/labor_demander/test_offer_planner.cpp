#include "components/labor_demander/common.hpp"
#include "components/labor_demander/mediator.hpp"
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

    SUBCASE("雇用計画が0でないとき、正しいログを返すこと") {
        constexpr auto employPlan   = HeadCount{100.0};
        constexpr auto employResult = HeadCount{150.0};
        constexpr auto plan =
            RecruitPlan{.wage = Wage{1}, .employ = employPlan, .offer = HeadCount{1}};
        constexpr auto result = RecruitResult{.applicants = HeadCount{1}, .employ = employResult};

        memory.listenRecruitPlan(plan);
        memory.listenRecruitResult(result);

        CHECK(memory.lastEmployResult()->value() == employResult.value());
        CHECK(memory.lastEmployPlan()->value() == employPlan.value());
    }

    SUBCASE("clearLogを呼び出した場合、logがいずれもnullであること") {
        memory.clearLog();
        CHECK(not memory.lastEmployPlan());
        CHECK(not memory.lastEmployResult());
    }
}

TEST_CASE("OfferPlannerのテスト") {  // NOLINT
    auto rng      = makeRng();
    auto planner  = OfferPlanner{rng};
    auto mediator = Mediator{};
    planner.acceptMediator(mediator);

    SUBCASE("mediateしない場合、2回目と1回目の結果が同じであること") {
        const auto first  = planner.plan(HeadCount{10.0});
        const auto second = planner.plan(HeadCount{10.0});

        CHECK(first.value() == second.value());
    }
}
}  // namespace abm::labor::demander::planner