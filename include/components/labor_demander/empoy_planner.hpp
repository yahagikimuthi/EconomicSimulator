#pragma once

#include <limits>
#include <optional>

#include "components/labor_demander/common.hpp"
#include "core/values/labor.hpp"
#include "setting.hpp"
#include "util.hpp"

namespace abm::labor::demander::planner {

// 要求雇用数を雇用計画に変換するための意味論的クラス
class EmployPlanner final {
  public:
    EmployPlanner() = delete;  // インスタンス化は想定しない
    [[nodiscard]] static auto plan(const HeadCount desiredEmploy) noexcept -> HeadCount {
        return desiredEmploy;
    }
};

class OfferPlannerMemory final {
  public:
    [[nodiscard]] explicit constexpr OfferPlannerMemory(RandomGenerator& masterRng) noexcept
        : applicants_{HeadCount{masterRng.random(setting::lastApplicants)}},
          employPlan_{HeadCount{masterRng.random(setting::lastEmployPlan)}} {}
    [[nodiscard]] auto lastApplicants() const noexcept -> std::optional<HeadCount> {
        return applicants_.log;
    }
    [[nodiscard]] auto lastEmployPlan() const noexcept -> std::optional<HeadCount> {
        return employPlan_.log;
    }
    void clearLog() noexcept { applicants_.clearLog(), employPlan_.clearLog(); }
    void reset() noexcept { applicants_.reset(), employPlan_.reset(); }
    void listenRecruitResult(const RecruitResult& result) noexcept {
        ASSERT(result.applicants >= HeadCount{0.0});
        applicants_.next = result.applicants;
    }

  private:
    Memory<HeadCount> applicants_;
    Memory<HeadCount> employPlan_;
};

class OfferPlanner final {
  public:
    [[nodiscard]] explicit constexpr OfferPlanner(RandomGenerator& masterRng) noexcept
        : memory_{masterRng},
          rateCache_{masterRng.random(setting::offerRate)},
          rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}},
          adjustVol_{masterRng.random(setting::offerRateAdjustVol)} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        mediator.subscribeRecruitResult(memory_);
    }

    [[nodiscard]] auto plan(const HeadCount employPlan) noexcept -> HeadCount {
        return HeadCount{employPlan.value() * (1.0 + planOfferRate())}.ceil();
    }

    void reset() noexcept {
        memory_.reset();
        rateCache_.reset();
    }

  private:
    [[nodiscard]] auto planOfferRate() noexcept -> double {
        const auto nextRate = calcOfferRate();
        memory_.clearLog();
        if (not nextRate) return rateCache_.cache();
        rateCache_.next(*nextRate);

        ASSERT(*nextRate > 0.0);
        return *nextRate;
    }

    [[nodiscard]] auto calcOfferRate() const noexcept -> std::optional<double> {
        const auto lastApplicants = memory_.lastApplicants();
        const auto lastEmployPlan = memory_.lastEmployPlan();
        if (not lastApplicants or not lastEmployPlan) return std::nullopt;
        const auto alpha       = rng_.randNormal(0.0, adjustVol_);
        const auto shouldRaise = *lastApplicants < *lastEmployPlan;
        const auto next        = rateCache_.cache() + (shouldRaise ? alpha : -alpha);
        return std::max(std::numeric_limits<double>::epsilon(), next);
    }

    OfferPlannerMemory      memory_;
    Cache<double>           rateCache_;
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};
}  // namespace abm::labor::demander::planner