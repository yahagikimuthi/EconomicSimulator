#include "components/labor_demander/planner.hpp"

#include <config.hpp>
#include <cstdint>
#include <limits>
#include <pcg_random.hpp>
#include <utility>

#include "core/values/labor.hpp"
#include "doctest.h"
#include "helper.hpp"
#include "tests/test_helper.hpp"

using test::equal;

namespace labor::demander {
class [[nodiscard]] RequestPlannerTester {
  public:
    RequestPlannerTester(const RequestPlanner& planner) : planner_{planner} {}
    auto employPlan() const -> HeadCount { return planner_.plan_.employ; }

  private:
    const RequestPlanner& planner_;
};

TEST_CASE("judgePlanのテスト") {  // NOLINT
    struct Log {
        HeadCount applicantNum;
        HeadCount offerPlan;
        Wage      wage;
    };
    struct Input {
        Log       log;
        HeadCount desiredEmploy;
        double    offerRate;
    };
    struct Expect {
        HeadCount             employPlan;
        HeadCount             offerPlan;
        std::pair<Wage, Wage> nextWageRange;
    };

    const std::uint64_t state{test::makeSeed()};
    const std::uint64_t stream{test::makeSeed()};

    pcg32 masterRng{state, stream};
    INFO("state: " << state);
    INFO("stream: " << stream);

    auto makeRequestPlanner{[&](const Input& input) -> RequestPlanner {
        pcg32           rng{masterRng(), masterRng()};
        const HeadCount lastEmploy{0.0};
        const double    wageAdjustVol{helper::rand(rng)};
        const double    offerAdjustVol{helper::rand(rng)};
        return {
            rng,
            input.log.wage,
            lastEmploy,
            input.log.offerPlan,
            input.log.applicantNum,
            input.offerRate,
            wageAdjustVol,
            offerAdjustVol
        };
    }};

    SUBCASE("賃金引き上げ+通常オファー計算") {
        const Input input{
            .log           = {.applicantNum = HeadCount{1}, .offerPlan{5}, .wage = Wage{100.0}},
            .desiredEmploy = HeadCount{10.0},
            .offerRate     = 0.05
        };
        const Expect expect{
            .employPlan    = HeadCount{10},
            .offerPlan     = HeadCount{11},
            .nextWageRange = {Wage{100.0}, Wage{200.0}}
        };
        RequestPlanner planner{makeRequestPlanner(input)};
        planner.judgePlan(input.desiredEmploy);

        RequestPlannerTester tester{planner};
        CHECK(equal(planner.offerPlan(), expect.offerPlan));
        CHECK(equal(tester.employPlan(), expect.employPlan));
        CHECK(planner.wagePlan() >= expect.nextWageRange.first);
        CHECK(planner.wagePlan() <= expect.nextWageRange.second);
    }

    SUBCASE("賃金引き下げ+等しい件数での判定") {
        const Input input{
            .log = {.applicantNum = HeadCount{5}, .offerPlan = HeadCount{5}, .wage = Wage{100.0}},
            .desiredEmploy = HeadCount{10},
            .offerRate     = 0.1
        };
        const Expect expect{
            .employPlan    = HeadCount{10},
            .offerPlan     = HeadCount{11},
            .nextWageRange = {Wage{100.0}, Wage{200.0}}
        };
        RequestPlanner planner{makeRequestPlanner(input)};

        planner.judgePlan(input.desiredEmploy);
        RequestPlannerTester tester{planner};

        CHECK(equal(tester.employPlan(), expect.employPlan));
        CHECK(planner.wagePlan() <= expect.nextWageRange.second);
        CHECK(planner.wagePlan() >= expect.nextWageRange.first);
        CHECK(equal(planner.offerPlan(), expect.offerPlan));
    }

    SUBCASE("上下限ガード") {
        const Input input{
            .log = {.applicantNum = HeadCount{10}, .offerPlan = HeadCount{0}, .wage = Wage{0.0005}},
            .desiredEmploy = HeadCount{90},
            .offerRate     = std::numeric_limits<double>::infinity()
        };
        const Expect expect{
            .employPlan    = HeadCount{90},
            .offerPlan     = HeadCount{config::agent_count::hhold},
            .nextWageRange = {Wage{config::labor_demander::epsilonWage}, Wage{0.0}}
        };
        RequestPlanner planner{makeRequestPlanner(input)};

        planner.judgePlan(input.desiredEmploy);
        RequestPlannerTester tester{planner};
        CHECK(equal(tester.employPlan(), expect.employPlan));
        CHECK(planner.wagePlan() >= expect.nextWageRange.first);
        CHECK(equal(planner.offerPlan(), expect.offerPlan));
    }
}

}  // namespace labor::demander