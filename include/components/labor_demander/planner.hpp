#pragma once

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/empoy_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"

namespace abm::labor::demander::planner {
class RecruitPlanner final {
  public:
    [[nodiscard]] explicit RecruitPlanner(RandomGenerator& masterRng)
        : wagePlanner_{masterRng}, employPlanSystem_{masterRng} {}

    [[nodiscard]] auto plan(const HeadCount desiredEmploy, IMediator auto& mediator)
        -> RecruitPlan {
        const auto plan = RecruitPlan{
            .wage = wagePlanner_.plan(), .offer = employPlanSystem_.plan(desiredEmploy, mediator)
        };
        return plan;
    }

    void commit() {
        wagePlanner_.commit();
        employPlanSystem_.commit();
    }

  private:
    WagePlanner          wagePlanner_;
    EmployPlanningSystem employPlanSystem_;
};
}  // namespace abm::labor::demander::planner

namespace abm::labor::demander {
using RecruitPlanner = planner::RecruitPlanner;
}