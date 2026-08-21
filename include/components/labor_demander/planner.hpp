#pragma once

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/empoy_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"

namespace abm::labor::demander::planner {
class RecruitPlanner final {
  public:
    [[nodiscard]] explicit constexpr RecruitPlanner(RandomGenerator& masterRng) noexcept
        : wagePlanner_{masterRng}, offerPlanner_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        wagePlanner_.acceptMediator(mediator);
        offerPlanner_.acceptMediator(mediator);
    }

    [[nodiscard]] auto plan(const HeadCount desiredEmploy, IMediator auto& mediator) noexcept
        -> RecruitPlan {
        ASSERT(desiredEmploy >= HeadCount{0.0});

        const auto wage   = wagePlanner_.plan();
        const auto employ = EmployPlanner::plan(desiredEmploy);
        const auto offer  = offerPlanner_.plan(employ);
        const auto plan   = RecruitPlan{.wage = wage, .offer = offer};

        mediator.publishEmployPlan(employ);
        mediator.publishRecruitPlan(plan);
        return plan;
    }

    void reset() noexcept {
        wagePlanner_.reset();
        offerPlanner_.reset();
    }

  private:
    WagePlanner  wagePlanner_;
    OfferPlanner offerPlanner_;
};
}  // namespace abm::labor::demander::planner

namespace abm::labor::demander {
using RecruitPlanner = planner::RecruitPlanner;
}