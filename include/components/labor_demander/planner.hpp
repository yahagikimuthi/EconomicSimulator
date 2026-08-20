#pragma once

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/empoy_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"

namespace abm::labor::demander::planner {
class RecruitPlanner final {
  public:
    [[nodiscard]] explicit RecruitPlanner(RandomGenerator& masterRng) noexcept
        : wagePlanner_{masterRng}, employPlanSystem_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        wagePlanner_.acceptMediator(mediator);
        employPlanSystem_.acceptMediator(mediator);
    }

    [[nodiscard]] auto plan(const HeadCount desiredEmploy, IMediator auto& mediator) noexcept
        -> RecruitPlan {
        ASSERT(desiredEmploy >= HeadCount{0.0});

        const auto plan = RecruitPlan{
            .wage = wagePlanner_.plan(), .offer = employPlanSystem_.plan(desiredEmploy, mediator)
        };
        mediator.publishRecruitPlan(plan);
        return plan;
    }

    void reset() noexcept {
        wagePlanner_.reset();
        employPlanSystem_.reset();
    }

  private:
    WagePlanner          wagePlanner_;
    EmployPlanningSystem employPlanSystem_;
};
}  // namespace abm::labor::demander::planner

namespace abm::labor::demander {
using RecruitPlanner = planner::RecruitPlanner;
}