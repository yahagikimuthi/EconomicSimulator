#include <limits>
#include "components/labor_demander/wage_planner.hpp"

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/mediator.hpp"
#include "doctest.h"
#include "others/setting.hpp"
#include "tests/util.hpp"
#include "values/labor.hpp"

namespace abm::labor::demander::planner {
namespace {
TEST_CASE("WagePlannerMemoryのテスト") {  // NOLINT
    auto rng    = makeRng();
    auto memory = WagePlannerMemory{rng};

    SUBCASE("雇用計画が0のとき、応募者数の記録を取らないこと") {
        constexpr auto plan =
            RecruitPlan{.wage = Wage{0.0}, .employ = HeadCount{0.0}, .offer = HeadCount{0.0}};
        const auto result = RecruitResult{.applicants = HeadCount{5.0}, .employ = HeadCount{1.0}};
        const auto lastEmployPlan = memory.lastEmployPlan();
        const auto lastApplicants = memory.lastApplicants();

        memory.listenRecruitPlan(plan);
        memory.listenRecruitResult(result);

        CHECK(memory.lastEmployPlan() == lastEmployPlan);
        CHECK(memory.lastApplicants() == lastApplicants);
    }

    SUBCASE("clearLogを呼び出した場合、logはnullを返すこと") {
        memory.clearLog();

        CHECK(not memory.lastEmployPlan());
        CHECK(not memory.lastApplicants());
    }
}

TEST_CASE("WagePlannerのテスト") {  // NOLINT
    auto rng      = makeRng();
    auto planner  = WagePlanner{rng};
    auto mediator = Mediator{};
    planner.acceptMediator(mediator);

    SUBCASE("mediateしない場合、2回目の結果が1回目と同じであること") {
        const auto first  = planner.plan(Money{10.0});
        const auto second = planner.plan(Money{100.0});

        CHECK(first.value() == doctest::Approx(second.value()));
    }

    SUBCASE("応募者数 < 雇用計画の場合、賃金が上がること") {
        constexpr auto infMoney = Money{std::numeric_limits<double>::infinity()};
        constexpr auto plan =
            RecruitPlan{.wage = Wage{1.0}, .employ = HeadCount{10.0}, .offer = HeadCount{10.0}};
        constexpr auto result =
            RecruitResult{.applicants = HeadCount{5.0}, .employ = HeadCount{5.0}};
        const auto first = planner.plan(infMoney);

        mediator.publishRecruitPlan(plan);
        mediator.publishRecruitResult(result);

        const auto second = planner.plan(infMoney);

        CHECK(second.value() > doctest::Approx(first.value()));
    }

    SUBCASE("賃金は1人あたり売上以下であること") {
        constexpr auto epsilon = Money{global_setting::epsilon * 2};
        const auto     wage    = planner.plan(epsilon);

        CHECK(wage.value() <= doctest::Approx(epsilon.value()));
    }
}
}  // namespace
}  // namespace abm::labor::demander::planner
