#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
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
    [[nodiscard]] auto wasApplicantsLack() const -> bool {
        return log_.applicants < log_.targetEmploy;
    }

    void memorizeWagePlan(Wage wagePlan) { plan_.wage = wagePlan; }
    void memorizeEmployPlan(HeadCount targetEmploy) { plan_.targetEmploy = targetEmploy; }
    void endStep(const RecruitResult& result) {
        ASSERT(plan_.wage and plan_.targetEmploy);
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
        const auto next = calcNextWage();
        memory_.memorizeWagePlan(next);
        memory_.memorizeEmployPlan(targetEmploy);
        return next;
    }

    void endStep(const RecruitResult& result) { memory_.endStep(result); }

  private:
    [[nodiscard]] auto calcNextWage() const -> Wage {
        const auto alpha = rng_.randNormal(0.0, adjustVol_, -1.0, 1.0);
        const auto raise = memory_.wasApplicantsLack();
        const auto nextWage =
            Wage{memory_.rememberLastWage() * (raise ? 1.0 + alpha : 1.0 - alpha)};
        return wageGuard(nextWage);
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
    [[nodiscard]] OfferPlannerMemory() = default;

    void memorizeEmployPlan(const HeadCount employPlan) { currentTargetEmploy_ = employPlan; }
    void endStep() {
        if (not currentTargetEmploy_) return;
        currentTargetEmploy_.reset();
    }

    [[nodiscard]] auto rememberEmployPlan() const -> HeadCount {
        ASSERT(currentTargetEmploy_.has_value());
        return *currentTargetEmploy_;
    }

  private:
    std::optional<HeadCount> currentTargetEmploy_{std::nullopt};
};

class OfferPlanner {
  public:
    [[nodiscard]] OfferPlanner(
        const RandomGenerator rng, const double offerRate, const double adjustVol
    )
        : rng_{rng}, offerRate_{offerRate}, adjustVol_{adjustVol} {}

    [[nodiscard]] auto plan(const HeadCount desiredEmploy) -> HeadCount {
        const auto next = calcNextOffer(desiredEmploy);
        memory_.memorizeEmployPlan(desiredEmploy);
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
        const auto alpha       = std::abs(rng_.randNormal(0.0, adjustVol_, -1.0, 1.0));
        const auto shouldRaise = actualEmploy < employPlan;
        const auto next        = offerRate_ * (shouldRaise ? 1.0 + alpha : 1.0 - alpha);
        return std::max(0.0, next);
    }

    OfferPlannerMemory      memory_;
    mutable RandomGenerator rng_;
    double                  offerRate_;
    const double            adjustVol_;
};

class RecruitPlanner {
  public:
    [[nodiscard]] RecruitPlanner(const WagePlanner& wagePlanner, const OfferPlanner& offerPlanner)
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

namespace abm::labor::demander {
using RecruitPlanner = planner::RecruitPlanner;
}