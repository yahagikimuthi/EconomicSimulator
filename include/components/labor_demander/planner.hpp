#pragma once

#include <cmath>
#include <limits>
#include <pcg_random.hpp>

#include "components/labor_demander/common.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"

namespace abm::labor::demander {
class WagePlanner {
  public:
    [[nodiscard]] WagePlanner(const RandomGenerator rng, const double adjustVol)
        : rng_{rng}, adjustVol_{adjustVol} {}

    [[nodiscard]] auto judgeWage(const RecruitPlan& lastPlan, const RecruitResult& lastResult) const
        -> Wage {
        const double alpha{rng_.randNormal(0.0, adjustVol_, -1.0, 1.0)};
        const bool   raise{shouldRaise(lastPlan.offer, lastResult.applicants)};
        const Wage   nextWage{lastPlan.wage * (raise ? 1.0 + alpha : 1.0 - alpha)};
        return wageGuard(nextWage);
    }

  private:
    [[nodiscard]] static auto shouldRaise(const HeadCount offerPlan, const HeadCount applicants)
        -> bool {
        return applicants < offerPlan;
    }

    [[nodiscard]] static auto wageGuard(const Wage wage) -> Wage {
        return Wage{std::max(wage.value(), std::numeric_limits<double>::epsilon())};
    }

    mutable RandomGenerator rng_;
    const double            adjustVol_;
};

class OfferPlanner {
  public:
    [[nodiscard]] OfferPlanner(
        const RandomGenerator rng, const double offerRate, const double adjustVol
    )
        : rng_{rng}, offerRate_{offerRate}, adjustVol_{adjustVol} {}

    [[nodiscard]] auto judgePlan(
        const RecruitPlan& lastPlan, const RecruitResult& lastResult, const HeadCount desiredEmploy
    ) -> HeadCount {
        offerRate_ = calcOfferRate(lastPlan.employ, lastResult.employ);
        return desiredEmploy * (1.0 + offerRate_);
    }

  private:
    [[nodiscard]] auto calcOfferRate(const HeadCount employPlan, const HeadCount actualEmploy) const
        -> double {
        const double alpha{std::abs(rng_.randNormal(0.0, adjustVol_, -1.0, 1.0))};
        const bool   shouldRaise{actualEmploy < employPlan};
        const double next{offerRate_ * (shouldRaise ? 1.0 + alpha : 1.0 - alpha)};
        return std::max(0.0, next);
    }

    mutable RandomGenerator rng_;
    double                  offerRate_;
    const double            adjustVol_;
};

class RequestPlanner {
  public:
    [[nodiscard]] RequestPlanner(const WagePlanner& wagePlanner, const OfferPlanner& offerPlanner)
        : wagePlanner_{wagePlanner}, offerPlanner_{offerPlanner} {}

    [[nodiscard]] auto judgePlan(
        const RecruitPlan& lastPlan, const RecruitResult& lastResult, const HeadCount desiredEmploy
    ) -> RecruitPlan {
        return {
            .wage   = wagePlanner_.judgeWage(lastPlan, lastResult),
            .employ = desiredEmploy,
            .offer  = offerPlanner_.judgePlan(lastPlan, lastResult, desiredEmploy)
        };
    }

  private:
    WagePlanner  wagePlanner_;
    OfferPlanner offerPlanner_;
};
}  // namespace abm::labor::demander