#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>

#include "components/labor_demander/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/labor.hpp"
#include "values/math.hpp"

namespace abm::labor::demander::planner {

// 要求雇用数を雇用計画に変換するための意味論的クラス
class EmployPlanner final {
  public:
    EmployPlanner() = delete;  // インスタンス化は想定しない
    [[nodiscard]] static auto plan(const HeadCount desiredEmploy) noexcept -> HeadCount {
        return desiredEmploy;
    }
};

// 前回雇用計画が必要
// 前回雇用結果中、雇用数が必要
class OfferPlannerMemory final {
  public:
    explicit OfferPlannerMemory(RandomGenerator& masterRng) noexcept
        : employResult_{HeadCount{masterRng.random(setting::lastApplicants)}},
          employPlan_{HeadCount{masterRng.random(setting::lastEmployPlan)}} {}
    [[nodiscard]] auto lastEmployResult() const noexcept -> std::optional<HeadCount> {
        return employResult_.log();
    }
    [[nodiscard]] auto lastEmployPlan() const noexcept -> std::optional<HeadCount> {
        return employPlan_.log();
    }
    void clearLog() noexcept { employResult_.clearLog(), employPlan_.clearLog(); }
    void reset() noexcept { employResult_.reset(), employPlan_.reset(); }
    void listenEmployPlan(const HeadCount employPlan) noexcept {
        ASSERT(employPlan >= HeadCount{0.0});
        employPlan_.next(employPlan);
    }
    void listenRecruitResult(const RecruitResult& result) noexcept {
        ASSERT(result.employ >= HeadCount{0.0});
        employResult_.next(result.applicants);
    }

  private:
    Memory<HeadCount> employResult_;
    Memory<HeadCount> employPlan_;
};

class OfferPlanner final {
  public:
    explicit OfferPlanner(RandomGenerator& masterRng) noexcept
        : memory_{masterRng},
          rateCache_{OfferRate{masterRng.random(setting::offerRate)}},
          rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}},
          adjustVol_{masterRng.random(setting::offerRateAdjustVol)} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        mediator.subscribeRecruitResult(memory_);
        mediator.subscribeEmployPlan(memory_);
    }

    [[nodiscard]] auto plan(const HeadCount employPlan) noexcept -> HeadCount {
        const auto out     = employPlan * (OfferRate{1.0} + planOfferRate());
        const auto guarded = std::min(out, HeadCount{::abm::setting::agent_count::hhold});
        return ceil(guarded);
    }

    void reset() noexcept {
        memory_.reset();
        rateCache_.reset();
    }

  private:
    [[nodiscard]] auto planOfferRate() noexcept -> OfferRate {
        const auto nextRate = calcOfferRate();
        memory_.clearLog();
        if (not nextRate) return rateCache_.cache();
        rateCache_.next(*nextRate);

        ASSERT(*nextRate > OfferRate{0.0});
        return *nextRate;
    }

    [[nodiscard]] auto calcOfferRate() const noexcept -> std::optional<OfferRate> {
        const auto lastEmployResult = memory_.lastEmployResult();
        const auto lastEmployPlan   = memory_.lastEmployPlan();
        if (not lastEmployResult or not lastEmployPlan) return std::nullopt;
        const auto alpha       = std::abs(rng_.randNormal(0.0, adjustVol_));
        const auto shouldRaise = *lastEmployResult < *lastEmployPlan;
        const auto next        = rateCache_.cache() + OfferRate{(shouldRaise ? alpha : -alpha)};
        const auto guarded     = std::clamp(
            next,
            OfferRate{std::numeric_limits<double>::epsilon()},
            OfferRate{::abm::setting::agent_count::hhold}
        );
        return guarded;
    }

    OfferPlannerMemory      memory_;
    Cache<OfferRate>        rateCache_;
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};
}  // namespace abm::labor::demander::planner