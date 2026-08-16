#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <pcg_random.hpp>

#include "components/labor_demander/common.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"

namespace abm::labor::demander::planner {
class WagePlannerMemory {
  public:
    [[nodiscard]] WagePlannerMemory(
        const Wage lastWage, const HeadCount lastTargetEmploy, const HeadCount lastApplicants
    )
        : log_{.wage = lastWage, .targetEmploy = lastTargetEmploy, .applicants = lastApplicants} {}
    [[nodiscard]] auto rememberLastWage() const -> Wage { return log_.wage; }
    [[nodiscard]] auto rememberLastTargetEmploy() const -> HeadCount { return log_.targetEmploy; }
    [[nodiscard]] auto rememberLastApplicants() const -> HeadCount { return log_.applicants; }

    void memorize(Wage wagePlan) { plan_.wage = wagePlan; }
    void memorize(HeadCount targetEmploy) { plan_.targetEmploy = targetEmploy; }
    void endStep(const RecruitResult& result) {
        ASSERT(plan_.wage.has_value() == plan_.targetEmploy.has_value());
        if (not plan_.wage) return;
        log_.applicants   = result.applicants;
        log_.wage         = *plan_.wage;
        log_.targetEmploy = *plan_.targetEmploy;
    }

  private:
    struct {
        Wage      wage;
        HeadCount targetEmploy;
        HeadCount applicants;
    } log_;

    struct {
        std::optional<Wage>      wage{std::nullopt};
        std::optional<HeadCount> targetEmploy{std::nullopt};
    } plan_{};
};

class WagePlanner {
  public:
    [[nodiscard]] WagePlanner(
        const RandomGenerator rng, const WagePlannerMemory& memory, const double adjustVol
    )
        : memory_{memory}, rng_{rng}, adjustVol_{adjustVol} {}

    [[nodiscard]] auto plan(const HeadCount targetEmploy) -> Wage {
        const auto next{calcNextWage()};
        memory_.memorize(next);
        memory_.memorize(targetEmploy);
        return next;
    }

    void endStep(const RecruitResult& result) { memory_.endStep(result); }

  private:
    [[nodiscard]] auto calcNextWage() const -> Wage {
        const auto alpha{rng_.randNormal(0.0, adjustVol_, -1.0, 1.0)};
        const auto raise{shouldRaise()};
        const auto nextWage{memory_.rememberLastWage() * (raise ? 1.0 + alpha : 1.0 - alpha)};
        return wageGuard(nextWage);
    }

    [[nodiscard]] auto shouldRaise() const -> bool {
        return memory_.rememberLastApplicants() < memory_.rememberLastTargetEmploy();
    }

    [[nodiscard]] static auto wageGuard(const Wage wage) -> Wage {
        return Wage{std::max(wage.value(), std::numeric_limits<double>::epsilon())};
    }

    WagePlannerMemory       memory_;
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};

class OfferPlannerMemory {
  public:
    [[nodiscard]] OfferPlannerMemory(const HeadCount targetEmploy, const HeadCount lastOffer)
        : log_{.targetEmploy = targetEmploy, .offer = lastOffer} {}

    void memorizeOfferPlan(const HeadCount offerPlan) { plan_.offer = offerPlan; }
    void memorizeEmployPlan(const HeadCount employPlan) { plan_.targetEmploy = employPlan; }
    void endStep() {
        ASSERT(plan_.offer.has_value() == plan_.targetEmploy.has_value());
        if (not plan_.offer.has_value()) return;
        log_.targetEmploy = *plan_.targetEmploy;
        log_.offer        = *plan_.offer;
    }

    [[nodiscard]] auto rememberLastOffer() const -> HeadCount { return log_.offer; }
    [[nodiscard]] auto rememberLastTargetEmploy() const -> HeadCount { return log_.targetEmploy; }
    [[nodiscard]] auto rememberEmployPlan() const -> HeadCount {
        ASSERT(plan_.targetEmploy);
        return *plan_.targetEmploy;
    }

  private:
    struct {
        HeadCount targetEmploy;
        HeadCount offer;
    } log_;

    struct {
        std::optional<HeadCount> targetEmploy{std::nullopt};
        std::optional<HeadCount> offer{std::nullopt};
    } plan_;
};

class OfferPlanner {
  public:
    [[nodiscard]] OfferPlanner(
        const RandomGenerator     rng,
        const OfferPlannerMemory& memory,
        const double              offerRate,
        const double              adjustVol
    )
        : memory_{memory}, rng_{rng}, offerRate_{offerRate}, adjustVol_{adjustVol} {}

    [[nodiscard]] auto plan(const HeadCount desiredEmploy) -> HeadCount {
        const auto next{calcNextOffer(desiredEmploy)};
        memory_.memorizeEmployPlan(desiredEmploy);
        memory_.memorizeOfferPlan(next);
        return next;
    }

    void endStep(const RecruitResult& result) {
        offerRate_ = calcOfferRate(memory_.rememberEmployPlan(), result.employ);
        memory_.endStep();
    }

  private:
    [[nodiscard]] auto calcNextOffer(const HeadCount desiredEmploy) const -> HeadCount {
        return desiredEmploy * (1.0 + offerRate_);
    }

    [[nodiscard]] auto calcOfferRate(const HeadCount employPlan, const HeadCount actualEmploy) const
        -> double {
        const double alpha{std::abs(rng_.randNormal(0.0, adjustVol_, -1.0, 1.0))};
        const bool   shouldRaise{actualEmploy < employPlan};
        const double next{offerRate_ * (shouldRaise ? 1.0 + alpha : 1.0 - alpha)};
        return std::max(0.0, next);
    }

    OfferPlannerMemory      memory_;
    mutable RandomGenerator rng_;
    double                  offerRate_;
    const double            adjustVol_;
};

class RequestPlanner {
  public:
    [[nodiscard]] RequestPlanner(const WagePlanner& wagePlanner, const OfferPlanner& offerPlanner)
        : wagePlanner_{wagePlanner}, offerPlanner_{offerPlanner} {}

    [[nodiscard]] auto plan(const HeadCount desiredEmploy) -> RecruitPlan {
        return {
            .wage = wagePlanner_.plan(desiredEmploy), .offer = offerPlanner_.plan(desiredEmploy)
        };
    }

    void endStep(const RecruitResult& result) {
        wagePlanner_.endStep(result);
        offerPlanner_.endStep(result);
    }

  private:
    WagePlanner  wagePlanner_;
    OfferPlanner offerPlanner_;
};
}  // namespace abm::labor::demander::planner