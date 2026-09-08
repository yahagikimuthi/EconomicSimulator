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
    explicit WagePlannerMemory(
        const Memory<HeadCount>& employPlan, const Memory<HeadCount>& applicants
    ) noexcept
        : employPlan_{employPlan}, applicants_{applicants} {}

    void listenRecruitPlan(const RecruitPlan& plan) noexcept {
        assert(plan.employ.isZeroOrMore());
        if (plan.employ.isPositive()) employPlan_.next(plan.employ);
    }
    void listenRecruitResult(const RecruitResult& result) noexcept {
        assert(result.applicants.isZeroOrMore());
        if (employPlan_.wasSetNext()) applicants_.next(result.applicants);
        employPlan_.reset();
        applicants_.reset();
    }
    [[nodiscard]] auto lastApplicants() const noexcept -> std::optional<HeadCount> {
        return applicants_.log();
    }
    [[nodiscard]] auto lastEmployPlan() const noexcept -> std::optional<HeadCount> {
        return employPlan_.log();
    }
    void clearLog() noexcept { employPlan_.clearLog(), applicants_.clearLog(); }

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
        assert(salesPerWorker.isZeroOrMore());
        const auto next = [&]() noexcept -> std::optional<Wage> {
            if (salesPerWorker <= Money{global_setting::epsilon})
                return calcWage(Money{std::numeric_limits<double>::infinity()});
            return calcWage(salesPerWorker);
        }();
        memory_.clearLog();
        if (not next) return cache_;
        cache_ = *next;

        assert(next->isZeroOrMore());
        return *next;
    }

  private:
    [[nodiscard]] auto calcWage(const Money salesPerWorker) const noexcept -> std::optional<Wage> {
        const auto lastApplicants = memory_.lastApplicants();
        const auto lastEmployPlan = memory_.lastEmployPlan();
        if (not lastApplicants or not lastEmployPlan) return std::nullopt;
        assert(not lastEmployPlan->isZero());

        const auto alpha     = std::abs(rng_.randNormal(0.0, adjustVol_, -1.0, 1.0));
        const auto raiseRate = [=]() noexcept -> double {
            const auto excess = *lastApplicants - *lastEmployPlan;
            if (excess.isPositive()) return 1.0 - alpha;
            if (excess.isZero()) return 1.0;
            return 1.0 + alpha;
        }();
        const auto plan    = cache_ * raiseRate;
        const auto guarded = std::min(plan, static_cast<Wage>(salesPerWorker));
        return wageGuard(guarded);
    }

    [[nodiscard]] static auto wageGuard(const Wage wage) noexcept -> Wage {
        return std::max(wage, Wage{std::numeric_limits<double>::epsilon()});
    }

    WagePlannerMemory       memory_;
    Wage                    cache_;
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};
}  // namespace abm::labor::demander::planner