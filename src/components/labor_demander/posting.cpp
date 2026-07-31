#include "components/labor_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <cmath>
#include <core/values/common.hpp>
#include <pcg_random.hpp>

#include "config.hpp"
#include "core/values/labor.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace {
[[nodiscard]] auto wageGuard(const Wage wage) -> Wage {
    return Wage{std::max(wage.value(), config::labor_demander::epsilonWage)};
}
}  // namespace

namespace labor_demander {
[[nodiscard]] auto RequestPlanner::calcNextWage() const -> Wage {
    const bool   shouldRaiseWage{log_.applicantNum < log_.offerPlan};
    const double alpha{std::abs(helper::randNormal(rng_, 0.0, param_.wageAdjustVol))};
    const Wage   nextWage{log_.wage * (shouldRaiseWage ? 1.0 + alpha : 1.0 - alpha)};
    return wageGuard(nextWage);
}

[[nodiscard]] auto RequestPlanner::calcNextOffer(const HeadCount employ) const -> HeadCount {
    const HeadCount offer{employ * (1.0 + param_.offerRate)};
    return HeadCount{std::round(offer.value())};
}

void RequestPlanner::judgePlan(const HeadCount desiredEmploy) {
    plan_ = {
        .wage = calcNextWage(), .employ = desiredEmploy, .offer = calcNextOffer(desiredEmploy)
    };
}

void Recruiter::post(
    const AgentID                                id,
    const HeadCount                              desiredEmploy,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    isRecruiting_ = true;
    planner_.judgePlan(desiredEmploy);
    if (planner_.offerPlan() == HeadCount{0.0}) return;
    isPosting_ = true;
    auto it{requestBox.emplace_back(id, planner_.wagePlan())};
    myRequest_ = &*it;
}
}  // namespace labor_demander