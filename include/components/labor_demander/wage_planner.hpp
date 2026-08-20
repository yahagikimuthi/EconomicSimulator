#pragma once

#include <algorithm>
#include <limits>
#include <optional>
#include <pcg_random.hpp>

#include "components/labor_demander/common.hpp"
#include "core/values/labor.hpp"
#include "setting.hpp"
#include "util.hpp"

namespace abm::labor::demander::planner {
class WagePlannerMemory final {
  public:
    [[nodiscard]] explicit WagePlannerMemory(RandomGenerator& masterRng) noexcept
        : employPlan_{HeadCount{masterRng.random(setting::lastEmployPlan)}},
          applicants_{HeadCount{masterRng.random(setting::lastApplicants)}} {}
    void listenEmployPlan(const HeadCount employPlan) noexcept {
        ASSERT(employPlan >= HeadCount{0.0});
        employPlan_.next = employPlan;
    }
    void listenRecruitResult(const RecruitResult& result) noexcept {
        ASSERT(result.applicants >= HeadCount{0.0});
        applicants_.next = result.applicants;
    }
    [[nodiscard]] auto lastApplicants() const noexcept -> std::optional<HeadCount> {
        return applicants_.log;
    }
    [[nodiscard]] auto lastEmployPlan() const noexcept -> std::optional<HeadCount> {
        return employPlan_.log;
    }
    void clearLog() noexcept { employPlan_.clearLog(), applicants_.clearLog(); }
    void reset() noexcept {
        employPlan_.reset();
        applicants_.reset();
    }

  private:
    Memory<HeadCount> employPlan_;
    Memory<HeadCount> applicants_;
};

class WagePlanner final {
  public:
    [[nodiscard]] explicit WagePlanner(RandomGenerator& masterRng) noexcept
        : memory_{masterRng},
          cache_{Wage{masterRng.random(setting::lastWage)}},
          rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}},
          adjustVol_{masterRng.random(setting::wageAdjustVol)} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        mediator.subscribeEmployPlan(memory_);
        mediator.subscribeRecruitResult(memory_);
    }

    [[nodiscard]] auto plan() noexcept -> Wage {
        const auto next = calcWage();
        memory_.clearLog();
        if (not next) return cache_.cache();
        cache_.next(*next);

        ASSERT(*next >= Wage{0.0});
        return *next;
    }

    void reset() noexcept {
        memory_.reset();
        cache_.reset();
    }

  private:
    [[nodiscard]] auto calcWage() const noexcept -> std::optional<Wage> {
        const auto lastApplicants = memory_.lastApplicants();
        const auto lastEmployPlan = memory_.lastEmployPlan();
        if (not lastApplicants or not lastEmployPlan) return std::nullopt;
        const auto alpha       = rng_.randNormal(0.0, adjustVol_, -1.0, 1.0);
        const auto shouldRaise = *lastApplicants < *lastEmployPlan;
        const auto plan        = Wage{cache_.cache() * (shouldRaise ? 1.0 + alpha : 1.0 - alpha)};
        return wageGuard(plan);
    }

    [[nodiscard]] static auto wageGuard(const Wage wage) noexcept -> Wage {
        return Wage{std::max(wage.value(), std::numeric_limits<double>::epsilon())};
    }

    WagePlannerMemory       memory_;
    Cache<Wage>             cache_;
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};
}  // namespace abm::labor::demander::planner