#pragma once

#include <optional>
#include <pcg_random.hpp>

#include "components/labor_demander/common.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"

namespace abm::labor::demander::planner {
class WagePlannerMemory {
    template <typename T>
    class Memory {
      public:
        [[nodiscard]] Memory(const T last) noexcept : last{last} {}
        void commit() noexcept {
            if (current) last = *current;
        }
        std::optional<T> last{std::nullopt};
        std::optional<T> current{std::nullopt};
    };

  public:
    [[nodiscard]] WagePlannerMemory(RandomGenerator& masterRng) noexcept
        : employPlan_{HeadCount{masterRng.rand(10, 20)}},
          applicants_{HeadCount{masterRng.rand(10, 20)}} {}

    void listenEmployPlan(const HeadCount employPlan) noexcept { employPlan_.current = employPlan; }

    void listenRecruitResult(const RecruitResult& result) noexcept {
        applicants_.current = result.applicants;
    }

    [[nodiscard]] auto rememberLastApplicants() const noexcept -> std::optional<HeadCount> {
        return applicants_.last;
    }
    [[nodiscard]] auto rememberLastEmployPlan() const noexcept -> std::optional<HeadCount> {
        return employPlan_.last;
    }

    void clearLog() noexcept { employPlan_.last.reset(), applicants_.last.reset(); }

    void commit() noexcept {
        employPlan_.commit();
        applicants_.commit();
    }

  private:
    Memory<HeadCount> employPlan_;
    Memory<HeadCount> applicants_;
};

class WagePlanner {
  public:
    [[nodiscard]] WagePlanner(RandomGenerator& rng) noexcept
        : lastWage_{rng.rand(10.0, 20.0)},
          memory_{rng},
          rng_{pcg32{rng.makeUint64(), rng.makeUint64()}},
          adjustVol_{rng.rand(0.1, 0.2)} {}

    [[nodiscard]] auto plan() noexcept -> Wage {
        const auto next = calcWage();
        memory_.clearLog();
        if (not next) return lastWage_;
        wagePlan_ = *next;
        return *next;
    }

    void commit() noexcept {
        memory_.commit();
        if (wagePlan_) lastWage_ = *wagePlan_;
    }

  private:
    [[nodiscard]] auto calcWage() const noexcept -> std::optional<Wage> {
        const auto lastApplicants = memory_.rememberLastApplicants();
        const auto lastEmployPlan = memory_.rememberLastEmployPlan();
        if (not lastApplicants or not lastEmployPlan) return std::nullopt;
        const auto alpha       = rng_.randNormal(0.0, adjustVol_, -1.0, 1.0);
        const auto shouldRaise = *lastApplicants < *lastEmployPlan;
        const auto plan        = Wage{lastWage_ * (shouldRaise ? 1.0 + alpha : 1.0 - alpha)};
        return wageGuard(plan);
    }

    [[nodiscard]] static auto wageGuard(const Wage wage) noexcept -> Wage {
        return Wage{std::max(wage.value(), std::numeric_limits<double>::epsilon())};
    }

    Wage                    lastWage_;
    std::optional<Wage>     wagePlan_;
    WagePlannerMemory       memory_;
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};
}  // namespace abm::labor::demander::planner