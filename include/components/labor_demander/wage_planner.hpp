#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>

#include "components/labor_demander/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/labor.hpp"

namespace abm::labor::demander::planner {
// 前回雇用計画が必要
// 前回雇用結果中、応募者数が必要
class WagePlannerMemory final {
  public:
    explicit WagePlannerMemory(RandomGenerator& masterRng) noexcept
        : employPlan_{HeadCount{masterRng.random(setting::lastEmployPlan)}},
          applicants_{HeadCount{masterRng.random(setting::lastApplicants)}} {}
    void listenRecruitPlan(const RecruitPlan& plan) noexcept {
        ASSERT(plan.employ.isZeroOrMore());
        employPlan_.next(plan.employ);
    }
    void listenRecruitResult(const RecruitResult& result) noexcept {
        ASSERT(result.applicants.isZeroOrMore());
        applicants_.next(result.applicants);
    }
    [[nodiscard]] auto lastApplicants() const noexcept -> std::optional<HeadCount> {
        return applicants_.log();
    }
    [[nodiscard]] auto lastEmployPlan() const noexcept -> std::optional<HeadCount> {
        return employPlan_.log();
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
    explicit WagePlanner(RandomGenerator& masterRng) noexcept
        : memory_{masterRng},
          cache_{Wage{masterRng.random(setting::lastWage)}},
          rng_{{masterRng.makeUint64(), masterRng.makeUint64()}},
          adjustVol_{masterRng.random(setting::wageAdjustVol)} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        mediator.subscribeRecruitPlan(memory_);
        mediator.subscribeRecruitResult(memory_);
    }

    [[nodiscard]] auto plan(const Money salesPerWorker) noexcept -> Wage {
        const auto next = [&]() noexcept -> std::optional<Wage> {
            if (salesPerWorker == Money{0.0}) {
                return calcWage(Money{std::numeric_limits<double>::infinity()});
            }
            return calcWage(salesPerWorker);
        }();
        memory_.clearLog();
        if (not next) return cache_.cache();
        cache_.next(*next);

        ASSERT(next->isZeroOrMore());
        return *next;
    }

    void reset() noexcept {
        memory_.reset();
        cache_.reset();
    }

  private:
    [[nodiscard]] auto calcWage(const Money salesPerWorker) const noexcept -> std::optional<Wage> {
        const auto lastApplicants = memory_.lastApplicants();
        const auto lastEmployPlan = memory_.lastEmployPlan();
        if (not lastApplicants or not lastEmployPlan) return std::nullopt;
        const auto alpha       = std::abs(rng_.randNormal(0.0, adjustVol_, -1.0, 1.0));
        const auto shouldRaise = *lastApplicants < *lastEmployPlan;
        const auto plan        = cache_.cache() * (shouldRaise ? 1.0 + alpha : 1.0 - alpha);
        const auto guarded     = std::min(plan, static_cast<Wage>(salesPerWorker));
        return wageGuard(guarded);
    }

    [[nodiscard]] static auto wageGuard(const Wage wage) noexcept -> Wage {
        return std::max(wage, Wage{std::numeric_limits<double>::epsilon()});
    }

    WagePlannerMemory       memory_;
    Cache<Wage>             cache_;
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};
}  // namespace abm::labor::demander::planner