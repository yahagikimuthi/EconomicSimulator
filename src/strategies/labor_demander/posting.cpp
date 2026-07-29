#include "components/labor_demander.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <cmath>
#include <config.hpp>
#include <pcg_random.hpp>

#include "helper.hpp"
#include "world/message.hpp"

namespace {
[[nodiscard]] auto wageGuard(const double wage) -> double {
    return std::max(wage, config::labor_demander::epsilonWage);
}
}  // namespace

namespace labor_demander {
[[nodiscard]] auto RequestPlanner::calcNextWage() const -> double {
    const bool   shouldRaiseWage{log_.applicantNum < log_.offerPlan};
    const double alpha{std::abs(helper::randNormal(rng_, 0.0, param_.wageAdjustVol))};
    const double nextWage{log_.wage * (shouldRaiseWage ? 1.0 + alpha : 1.0 - alpha)};
    return wageGuard(nextWage);
}

[[nodiscard]] auto RequestPlanner::calcNextOffer(const int employ) const -> int {
    const double offer{employ * (1.0 + param_.offerRate)};
    return static_cast<int>(std::round(offer));
}

void RequestPlanner::judgePlan(const int desiredEmploy) {
    plan_ = {
        .wage = calcNextWage(), .employ = desiredEmploy, .offer = calcNextOffer(desiredEmploy)
    };
}

void Recruiter::post(
    const int id, const int desiredEmploy, tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    isRecruiting_ = true;
    planner_.judgePlan(desiredEmploy);
    if (planner_.offerPlan() == 0) return;
    isPosting_ = true;
    auto it{requestBox.emplace_back(id, planner_.wagePlan())};
    myRequest_ = &*it;
}
}  // namespace labor_demander