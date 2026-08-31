#pragma once

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/empoy_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"
#include "others/util.hpp"
#include "values/labor.hpp"

namespace abm::labor::demander::planner {
class RecruitPlanner final {
  public:
    explicit RecruitPlanner(RandomGenerator& masterRng) noexcept
        : wagePlanner_{masterRng}, offerPlanner_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        wagePlanner_.acceptMediator(mediator);
        offerPlanner_.acceptMediator(mediator);
    }

    [[nodiscard]] auto plan(const HeadCount desiredEmploy, const Money salesPerWorker) noexcept
        -> RecruitPlan {
        ASSERT(desiredEmploy.isZeroOrMore());
        ASSERT(salesPerWorker >= Money{0.0});

        const auto wage   = wagePlanner_.plan(salesPerWorker);
        const auto employ = EmployPlanner::plan(desiredEmploy);
        const auto offer  = offerPlanner_.plan(employ);
        const auto plan   = RecruitPlan{.wage = wage, .employ = employ, .offer = offer};

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