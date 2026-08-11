#pragma once

#include <cmath>
#include <pcg_random.hpp>

#include "core/values/labor.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace labor::demander::internal {
[[nodiscard]] inline auto wageGuard(const Wage wage) -> Wage {
    return Wage{std::max(wage.value(), config::labor_demander::epsilonWage)};
}
}  // namespace labor::demander::internal

namespace labor::demander {
class [[nodiscard]] RequestPlanner {
  public:
    RequestPlanner(
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
              .offerRate      = offerRate,
              .wageAdjustVol  = wageAdjustVol,
              .offerAdjustVol = offerAdjustVol
          } {}

    void judgePlan(const HeadCount desiredEmploy) {
        plan_ = {
            .wage = calcNextWage(), .employ = desiredEmploy, .offer = calcNextOffer(desiredEmploy)
        };
    }

    auto wagePlan() const -> Wage POST(wage : wage > Wage{0.0}) { return plan_.wage; }
    auto offerPlan() const -> HeadCount POST(employ : employ >= HeadCount{0.0}) {
        return plan_.offer;
    }

    void endStep(CensusDropBox& dropBox, const HeadCount actualEmploy, const HeadCount applicantNum)
        PRE(actualEmploy >= HeadCount{0.0}) PRE(applicantNum >= HeadCount{0.0}) {
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

  private:
    auto calcNextWage() const -> Wage POST(wage : wage > Wage{0.0}) {
        const bool   shouldRaiseWage{log_.applicantNum < log_.offerPlan};
        const double alpha{std::abs(helper::randNormal(rng_, 0.0, param_.wageAdjustVol, -1.0, 1.0))
        };
        const Wage   nextWage{log_.wage * (shouldRaiseWage ? 1.0 + alpha : 1.0 - alpha)};
        return internal::wageGuard(nextWage);
    }

    auto calcNextOffer(const HeadCount desiredEmploy) const -> HeadCount
        POST(offer
             : offer >= HeadCount{0.0}) {
        const HeadCount offer{desiredEmploy * (1.0 + param_.offerRate)};
        return HeadCount{
            std::min(static_cast<double>(config::agent_count::hhold), std::ceil(offer.value()))
        };
    }
    auto updateOfferRate(const HeadCount actualEmploy) const -> double POST(rate : rate > 0.0) {
        const double alpha{std::abs(helper::randNormal(rng_, 0.0, param_.offerAdjustVol, -1.0, 1.0))
        };
        const bool   shouldRaise{actualEmploy < plan_.employ};
        const double offerRate{param_.offerRate * (shouldRaise ? 1.0 + alpha : 1.0 - alpha)};
        return std::max(1.0, offerRate);
    }

    mutable pcg32 rng_;
    struct {
        Wage      wage{0.0};
        HeadCount employ{0.0};
        HeadCount offer{0.0};
        void      reset() { wage = Wage{0.0}, employ = HeadCount{0.0}, offer = HeadCount{0.0}; }
    } plan_{};

    struct {
        Wage      wage;
        HeadCount actualEmploy;
        HeadCount offerPlan;
        HeadCount applicantNum;
    } log_;

    struct {
        double       offerRate;
        const double wageAdjustVol;
        const double offerAdjustVol;
    } param_;
};
}  // namespace labor::demander