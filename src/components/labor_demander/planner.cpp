#include "components/labor_demander/planner.hpp"

#include <cmath>

#include "core/values/labor.hpp"
#include "helper.hpp"

namespace {
[[nodiscard]] auto wageGuard(const Wage wage) -> Wage {
    return Wage{std::max(wage.value(), config::labor_demander::epsilonWage)};
}
}  // namespace

namespace labor::demander {
RequestPlanner::RequestPlanner(
    const pcg32     rng,
    const Wage      lastWage,
    const HeadCount lastEmploy,
    const HeadCount lastOfferPlan,
    const HeadCount lastApplicantNum,
    const double    offerRate,
    const double    wageAdjustVol,
    const double    offerAdjustVol
)
    : rng_{rng},
      log_{
          .wage         = lastWage,
          .actualEmploy = lastEmploy,
          .offerPlan    = lastOfferPlan,
          .applicantNum = lastApplicantNum
      },
      param_{
          .offerRate = offerRate, .wageAdjustVol = wageAdjustVol, .offerAdjustVol = offerAdjustVol
      } {}

[[nodiscard]] auto RequestPlanner::calcNextWage() const -> Wage {
    const bool   shouldRaiseWage{log_.applicantNum < log_.offerPlan};
    const double alpha{std::abs(helper::randNormal(rng_, 0.0, param_.wageAdjustVol, -1.0, 1.0))};
    const Wage   nextWage{log_.wage * (shouldRaiseWage ? 1.0 + alpha : 1.0 - alpha)};
    return wageGuard(nextWage);
}

[[nodiscard]] auto RequestPlanner::calcNextOffer(const HeadCount employ) const -> HeadCount {
    const HeadCount offer{employ * (1.0 + param_.offerRate)};
    return HeadCount{
        std::min(static_cast<double>(config::agent_count::hhold), std::ceil(offer.value()))
    };
}

void RequestPlanner::judgePlan(const HeadCount desiredEmploy) {
    plan_ = {
        .wage = calcNextWage(), .employ = desiredEmploy, .offer = calcNextOffer(desiredEmploy)
    };
}

auto RequestPlanner::updateOfferRate(const HeadCount actualEmploy) const -> double {
    const double alpha{std::abs(helper::randNormal(rng_, 0.0, param_.offerAdjustVol, -1.0, 1.0))};
    const bool   shouldRaise{actualEmploy < plan_.employ};
    const double offerRate{param_.offerRate * (shouldRaise ? 1.0 + alpha : 1.0 - alpha)};
    return std::max(0.0, offerRate);
}


void RequestPlanner::endStep(
    world::CensusDropBox& dropBox, const HeadCount actualEmploy, const HeadCount applicantNum
) {
    dropBox.postedEmployments.emplace_back(plan_.employ.value());
    log_ = {
        .wage         = plan_.wage,
        .actualEmploy = actualEmploy,
        .offerPlan    = plan_.offer,
        .applicantNum = applicantNum
    };
    param_.offerRate = updateOfferRate(actualEmploy);
    plan_.reset();
}
}  // namespace labor::demander