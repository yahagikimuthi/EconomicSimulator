#pragma once

#include <optional>
#include <pcg_random.hpp>

#include "components/labor_demander/common.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"

namespace abm::labor::demander::planner {
class WagePlannerMemory final {
  public:
    [[nodiscard]] explicit WagePlannerMemory(RandomGenerator& masterRng) noexcept
        : employPlan_{HeadCount{masterRng.rand(10, 20)}},
          applicants_{HeadCount{masterRng.rand(10, 20)}} {}
    void listenEmployPlan(const HeadCount employPlan) noexcept { employPlan_.next = employPlan; }
    void listenRecruitResult(const RecruitResult& result) noexcept {
        applicants_.next = result.applicants;
    }
    [[nodiscard]] auto rememberLastApplicants() const noexcept -> std::optional<HeadCount> {
        return applicants_.log;
    }
    [[nodiscard]] auto rememberLastEmployPlan() const noexcept -> std::optional<HeadCount> {
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
    friend class ::abm::labor::demander::LaborDemanderFactory;

  public:
    [[nodiscard]] explicit WagePlanner(RandomGenerator& rng) noexcept
        : cache_{Wage{rng.rand(10.0, 20.0)}},
          memory_{rng},
          rng_{pcg32{rng.makeUint64(), rng.makeUint64()}},
          adjustVol_{rng.rand(0.1, 0.2)} {}

    [[nodiscard]] auto plan() noexcept -> Wage {
        const auto next = calcWage();
        memory_.clearLog();
        if (not next) return cache_.cache();
        cache_.next(*next);
        return *next;
    }

    void reset() noexcept {
        memory_.reset();
        cache_.reset();
    }

  private:
    [[nodiscard]] auto calcWage() const noexcept -> std::optional<Wage> {
        const auto lastApplicants = memory_.rememberLastApplicants();
        const auto lastEmployPlan = memory_.rememberLastEmployPlan();
        if (not lastApplicants or not lastEmployPlan) return std::nullopt;
        const auto alpha       = rng_.randNormal(0.0, adjustVol_, -1.0, 1.0);
        const auto shouldRaise = *lastApplicants < *lastEmployPlan;
        const auto plan        = Wage{cache_.cache() * (shouldRaise ? 1.0 + alpha : 1.0 - alpha)};
        return wageGuard(plan);
    }

    [[nodiscard]] static auto wageGuard(const Wage wage) noexcept -> Wage {
        return Wage{std::max(wage.value(), std::numeric_limits<double>::epsilon())};
    }

    Cache<Wage>             cache_;
    WagePlannerMemory       memory_;
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};
}  // namespace abm::labor::demander::planner