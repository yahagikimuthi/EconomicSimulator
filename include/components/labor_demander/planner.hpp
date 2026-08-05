#pragma once

#include <pcg_random.hpp>

#include "core/values/labor.hpp"
#include "world/message.hpp"

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
    );

    void judgePlan(const HeadCount desiredEmploy);
    auto wagePlan() const -> Wage POST(wage : wage > Wage{0.0}) { return plan_.wage; }
    auto offerPlan() const -> HeadCount POST(employ : employ >= HeadCount{0.0}) {
        return plan_.offer;
    }
    void endStep(
        world::CensusDropBox& dropBox, const HeadCount actualEmploy, const HeadCount applicantNum
    ) PRE(actualEmploy >= HeadCount{0.0}) PRE(applicantNum >= HeadCount{0.0});

  private:
    auto calcNextWage() const -> Wage POST(wage : wage > Wage{0.0});
    auto calcNextOffer(const HeadCount desiredEmploy) const -> HeadCount
        POST(offer
             : offer >= HeadCount{0.0});
    auto updateOfferRate(const HeadCount actualEmploy) const -> double POST(rate : rate > 0.0);

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

    friend class RequestPlannerTester;
};
}  // namespace labor::demander