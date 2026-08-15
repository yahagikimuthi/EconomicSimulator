#pragma once

#include <cmath>
#include <limits>
#include <pcg_random.hpp>

#include "components/labor_demander/util.hpp"
#include "components/util.hpp"
#include "core/values/labor.hpp"

namespace abm::labor::demander {
class [[nodiscard]] WagePlanner {
  public:
    WagePlanner(const pcg32 rng, const double adjustVol) : rng_{rng}, adjustVol_{adjustVol} {}

    auto judgeWage(const RecruitPlan& lastPlan, const RecruitmentResult& lastRecruitment) const
        -> Wage {
        const Wage nextWage{
            calcWage(shouldRaise(lastPlan.offer, lastRecruitment.applicants), lastPlan.wage)
        };
        return nextWage;
    }

  private:
    auto calcWage(const bool shouldRaise, const Wage& lastWage) const -> Wage {
        const double alpha{rng_.randNormal(0.0, adjustVol_, -1.0, 1.0)};
        const Wage   nextWage{lastWage * (shouldRaise ? 1.0 + alpha : 1.0 - alpha)};
        return wageGuard(nextWage);
    }

    static auto shouldRaise(const HeadCount offerPlan, const HeadCount applicants) -> bool {
        return applicants < offerPlan;
    }

    static auto wageGuard(const Wage wage) -> Wage {
        return Wage{std::max(wage.value(), std::numeric_limits<double>::epsilon())};
    }

    mutable detail::RandomGenerator rng_;
    const double                    adjustVol_;
};

class [[nodiscard]] OfferPlanner {
  public:
    OfferPlanner(const pcg32 rng, const double offerRate, const double adjustVol)
        : rng_{rng}, offerRate_{offerRate}, adjustVol_{adjustVol} {}

    auto judgeOffer(
        const RecruitPlan&       lastPlan,
        const RecruitmentResult& lastRecruitment,
        const HeadCount          desiredEmploy
    ) -> HeadCount {
        offerRate_ = calcOfferRate(lastPlan.employ, lastRecruitment.employ);
        return desiredEmploy * (1.0 + offerRate_);
    }

  private:
    auto calcOfferRate(const HeadCount employPlan, const HeadCount actualEmploy) const -> double {
        const double alpha{std::abs(rng_.randNormal(0.0, adjustVol_, -1.0, 1.0))};
        const bool   shouldRaise{actualEmploy < employPlan};
        const double next{offerRate_ * (shouldRaise ? 1.0 + alpha : 1.0 - alpha)};
        return std::max(0.0, next);
    }

    mutable detail::RandomGenerator rng_;
    double                          offerRate_;
    const double                    adjustVol_;
};

class [[nodiscard]] RequestPlanner {
  public:
    RequestPlanner(const WagePlanner& wagePlanner, const OfferPlanner& offerPlanner)
        : wagePlanner_{wagePlanner}, offerPlanner_{offerPlanner} {}

    auto judgePlan(
        const RecruitPlan&       lastPlan,
        const RecruitmentResult& lastRecruitment,
        const HeadCount          desiredEmploy
    ) -> RecruitPlan {
        return {
            .wage   = wagePlanner_.judgeWage(lastPlan, lastRecruitment),
            .employ = desiredEmploy,
            .offer  = offerPlanner_.judgeOffer(lastPlan, lastRecruitment, desiredEmploy)
        };
    }

  private:
    WagePlanner  wagePlanner_;
    OfferPlanner offerPlanner_;
};
}  // namespace abm::labor::demander