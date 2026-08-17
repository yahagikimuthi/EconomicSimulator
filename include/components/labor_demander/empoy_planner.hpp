#pragma once

#include <optional>

#include "components/labor_demander/common.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"

namespace abm::labor::demander::planner {
class OfferPlanner {
  public:
    [[nodiscard]] OfferPlanner(RandomGenerator& masterRng)
        : rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}},
          adjustVol_{masterRng.rand(0.1, 0.2)} {}

    [[nodiscard]] auto plan(const HeadCount desiredEmploy, const double offerRate) const
        -> HeadCount {
        return desiredEmploy * offerRate;
    }

  private:
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};

struct JudgeOfferRateContext {
    const double    lastOfferRate;
    const HeadCount employPlan;
    const HeadCount applicants;
};

class EmployPlannerMemory {
  public:
    [[nodiscard]] EmployPlannerMemory(RandomGenerator& masterRng)
        : lastOfferRate_{masterRng.rand()} {}

    void memorizeEmployPlan(const HeadCount employPlan) { currentEmployPlan_ = employPlan; }
    void memorizeOfferRate(const double rate) { currentOfferRate_ = rate; }
    void listenRecruitResult(const RecruitResult& result) {
        currentApplicants_ = result.applicants;
    }

    [[nodiscard]] auto makeJudgeOfferRateContext() const -> JudgeOfferRateContext {
        ASSERT(currentEmployPlan_);
        ASSERT(currentApplicants_);
        return {
            .lastOfferRate = lastOfferRate_,
            .employPlan    = *currentEmployPlan_,
            .applicants    = *currentApplicants_
        };
    }
    [[nodiscard]] auto rememberOfferRate() const -> double { return lastOfferRate_; }

    void commit() {
        currentApplicants_.reset();
        currentEmployPlan_.reset();
        if (currentOfferRate_) lastOfferRate_ = *currentOfferRate_;
    }

  private:
    std::optional<HeadCount> currentApplicants_{std::nullopt};
    std::optional<HeadCount> currentEmployPlan_{std::nullopt};
    double                   lastOfferRate_;
    std::optional<double>    currentOfferRate_{std::nullopt};
};

class OfferRate {
  public:
    [[nodiscard]] OfferRate(RandomGenerator& rng)
        : rng_{pcg32{rng.makeUint64(), rng.makeUint64()}}, adjustVol_{rng.rand(0.1, 0.2)} {}

    [[nodiscard]] auto updateOfferRate(const JudgeOfferRateContext& ctx) const -> double {
        const auto alpha       = rng_.randNormal(0.0, adjustVol_);
        const auto shouldRaise = ctx.applicants < ctx.employPlan;
        const auto out         = ctx.lastOfferRate + (shouldRaise ? alpha : -alpha);
        return std::max(0.0, out);
    }

  private:
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};

class EmployPlanningSystem {
  public:
    [[nodiscard]] EmployPlanningSystem(RandomGenerator& masterRng)
        : offerPlanner_{masterRng}, memory_{masterRng}, offerRate_{masterRng} {}

    [[nodiscard]] auto plan(const HeadCount desiredEmploy, IMediator auto& mediator) -> HeadCount {
        const auto offerPlan = offerPlanner_.plan(desiredEmploy, memory_.rememberOfferRate());
        mediator.publishEmployPlan(desiredEmploy);
        return offerPlan;
    }

    void endStep() {
        const auto newRate = offerRate_.updateOfferRate(memory_.makeJudgeOfferRateContext());
        memory_.memorizeOfferRate(newRate);
    }

    void commit() { memory_.commit(); }

  private:
    OfferPlanner        offerPlanner_;
    EmployPlannerMemory memory_;
    OfferRate           offerRate_;
};
}  // namespace abm::labor::demander::planner