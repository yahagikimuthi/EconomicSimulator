#include "components/labor_demander/offer_planner.hpp"

#include <limits>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/mediator.hpp"
#include "doctest.h"
#include "tests/util.hpp"
#include "values/labor.hpp"

namespace abm::labor::demander::planner {
namespace {
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
    constexpr auto laborSupplier = HeadCount{std::numeric_limits<double>::infinity()};

    SUBCASE("mediateしない場合、2回目と1回目のオファー数が同じであること") {
        constexpr auto employ = HeadCount{10.0};

        const auto first  = planner.plan(employ, laborSupplier);
        const auto second = planner.plan(employ, laborSupplier);

        CHECK(first.value() == doctest::Approx(second.value()));
    }

    SUBCASE("雇用数 < 雇用計画の場合、オファー率が上がること") {
        constexpr auto inEmploy = HeadCount{1000.0};
        constexpr auto plan =
            RecruitPlan{.wage = Wage{1.0}, .employ = HeadCount{10.0}, .offer = HeadCount{20.0}};
        constexpr auto result =
            RecruitResult{.applicants = HeadCount{15.0}, .employ = HeadCount{5.0}};

        const auto beforePlan = planner.plan(inEmploy, laborSupplier);

        mediator.publishRecruitPlan(plan);
        mediator.publishRecruitResult(result);

        const auto afterPlan = planner.plan(inEmploy, laborSupplier);

        CHECK(afterPlan.value() > doctest::Approx(beforePlan.value()));
    }

    SUBCASE("雇用数 == 雇用計画の場合、オファー率が変わらないこと") {
        constexpr auto inEmploy = HeadCount{1000.0};
        constexpr auto plan =
            RecruitPlan{.wage = Wage{1.0}, .employ = HeadCount{10.0}, .offer = HeadCount{20.0}};
        constexpr auto result =
            RecruitResult{.applicants = HeadCount{15.0}, .employ = HeadCount{10.0}};

        const auto beforePlan = planner.plan(inEmploy, laborSupplier);

        mediator.publishRecruitPlan(plan);
        mediator.publishRecruitResult(result);

        const auto afterPlan = planner.plan(inEmploy, laborSupplier);

        CHECK(afterPlan.value() == doctest::Approx(beforePlan.value()));
    }

    SUBCASE("雇用数 > 雇用計画の場合、オファー率が下がることこと") {
        constexpr auto inEmploy = HeadCount{10000.0};

        const auto beforePlan = planner.plan(inEmploy, laborSupplier);
        CHECK(not beforePlan.isZero());

        mediator.publishRecruitPlan(
            RecruitPlan{.wage = Wage{1}, .employ = inEmploy, .offer = beforePlan}
        );
        mediator.publishRecruitResult(RecruitResult{
            .applicants = HeadCount{rng.randInt(0, 1000)}, .employ = HeadCount{inEmploy * 2}
        });

        const auto afterPlan = planner.plan(inEmploy, laborSupplier);

        CHECK(afterPlan.value() < doctest::Approx(beforePlan.value()));
    }
}
}  // namespace
}  // namespace abm::labor::demander::planner