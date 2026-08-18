#pragma once

#include <optional>

#include "components/labor_demander/common.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"

namespace abm::labor::demander::planner {
class EmployPlanner final {
  public:
    [[nodiscard]] auto plan(const HeadCount desiredEmploy) const noexcept -> HeadCount {
        return desiredEmploy;
    }
};

class OfferPlannerMemory final {
    template <typename T>
    class Memory final {
      public:
        [[nodiscard]] Memory(const T Last) noexcept : last{Last} {}

        void commit() noexcept {
            if (current) last = *current;
        }

        std::optional<T> last;
        std::optional<T> current;
    };

  public:
    [[nodiscard]] OfferPlannerMemory(RandomGenerator& masterRng) noexcept
        : applicants_{HeadCount{masterRng.rand(10, 20)}},
          employPlan_{HeadCount{masterRng.rand(10, 20)}} {}
    [[nodiscard]] auto rememberLastApplicants() const noexcept -> std::optional<HeadCount> {
        return applicants_.last;
    }
    [[nodiscard]] auto rememberLastEmployPlan() const noexcept -> std::optional<HeadCount> {
        return employPlan_.last;
    }
    void clearLog() noexcept { applicants_.last.reset(), employPlan_.last.reset(); }
    void commit() noexcept { applicants_.commit(), employPlan_.commit(); }
    void listenRecruitResult(const RecruitResult& result) noexcept {
        applicants_.current = result.applicants;
    }

  private:
    Memory<HeadCount> applicants_;
    Memory<HeadCount> employPlan_;
};

class OfferPlanner final {
  public:
    [[nodiscard]] OfferPlanner(RandomGenerator& rng) noexcept
        : memory_{rng},
          offerRate_{rng.rand(0.1, 0.2)},
          rng_{pcg32{rng.makeUint64(), rng.makeUint64()}},
          adjustVol_{rng.rand(0.1, 0.2)} {}

    [[nodiscard]] auto plan(const HeadCount employPlan) noexcept -> HeadCount {
        return HeadCount{employPlan * (1.0 + planOfferRate())};
    }

    void commit() noexcept {
        memory_.commit();
        if (currentOfferRate_) offerRate_ = *currentOfferRate_;
    }

  private:
    [[nodiscard]] auto planOfferRate() noexcept -> double {
        const auto nextRate = calcOfferRate();
        memory_.clearLog();
        if (not nextRate) return offerRate_;
        currentOfferRate_ = nextRate;
        return *nextRate;
    }

    [[nodiscard]] auto calcOfferRate() const -> std::optional<double> {
        const auto lastApplicants = memory_.rememberLastApplicants();
        const auto lastEmployPlan = memory_.rememberLastEmployPlan();
        if (not lastApplicants or not lastEmployPlan) return std::nullopt;
        const auto alpha       = rng_.randNormal(0.0, adjustVol_);
        const auto shouldRaise = *lastApplicants < *lastEmployPlan;
        const auto next        = offerRate_ + (shouldRaise ? alpha : -alpha);
        return std::max(0.0, next);
    }

    OfferPlannerMemory      memory_;
    double                  offerRate_;
    std::optional<double>   currentOfferRate_;
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};

class EmployPlanningSystem final {
  public:
    [[nodiscard]] EmployPlanningSystem(RandomGenerator& masterRng) : offerPlanner_{masterRng} {}

    [[nodiscard]] auto plan(const HeadCount desiredEmploy, IMediator auto& mediator) -> HeadCount {
        const auto employPlan = employPlanner_.plan(desiredEmploy);
        mediator.publishEmployPlan(employPlan);
        const auto offerPlan = offerPlanner_.plan(employPlan);
        return offerPlan;
    }

    void commit() { offerPlanner_.commit(); }

  private:
    EmployPlanner employPlanner_;
    OfferPlanner  offerPlanner_;
};
}  // namespace abm::labor::demander::planner