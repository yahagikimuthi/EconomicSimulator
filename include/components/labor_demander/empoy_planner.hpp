#pragma once

#include <optional>

#include "components/labor_demander/common.hpp"
#include "config.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"

namespace abm::labor::demander::planner {
class EmployPlanner final {
  public:
    [[nodiscard]] static auto plan(const HeadCount desiredEmploy) noexcept -> HeadCount {
        return desiredEmploy;
    }
};

class OfferPlannerMemory final {
  public:
    [[nodiscard]] explicit OfferPlannerMemory(RandomGenerator& masterRng) noexcept
        : applicants_{HeadCount{masterRng.random(config::lastApplicants)}},
          employPlan_{HeadCount{masterRng.random(config::lastEmployPlan)}} {}
    [[nodiscard]] auto rememberLastApplicants() const noexcept -> std::optional<HeadCount> {
        return applicants_.log;
    }
    [[nodiscard]] auto rememberLastEmployPlan() const noexcept -> std::optional<HeadCount> {
        return employPlan_.log;
    }
    void clearLog() noexcept { applicants_.clearLog(), employPlan_.clearLog(); }
    void reset() noexcept { applicants_.reset(), employPlan_.reset(); }
    void listenRecruitResult(const RecruitResult& result) noexcept {
        applicants_.next = result.applicants;
    }

  private:
    Memory<HeadCount> applicants_;
    Memory<HeadCount> employPlan_;
};

class OfferPlanner final {
  public:
    [[nodiscard]] explicit OfferPlanner(RandomGenerator& masterRng) noexcept
        : memory_{masterRng},
          rateCache_{masterRng.random(config::offerRate)},
          rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}},
          adjustVol_{masterRng.random(config::offerRateAdjustVol)} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        mediator.subscribeRecruitResult(memory_);
    }

    [[nodiscard]] auto plan(const HeadCount employPlan) noexcept -> HeadCount {
        return HeadCount{employPlan * (1.0 + planOfferRate())};
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
        return *nextRate;
    }

    [[nodiscard]] auto calcOfferRate() const noexcept -> std::optional<double> {
        const auto lastApplicants = memory_.rememberLastApplicants();
        const auto lastEmployPlan = memory_.rememberLastEmployPlan();
        if (not lastApplicants or not lastEmployPlan) return std::nullopt;
        const auto alpha       = rng_.randNormal(0.0, adjustVol_);
        const auto shouldRaise = *lastApplicants < *lastEmployPlan;
        const auto next        = rateCache_.cache() + (shouldRaise ? alpha : -alpha);
        return std::max(0.0, next);
    }

    OfferPlannerMemory      memory_;
    Cache<double>           rateCache_;
    mutable RandomGenerator rng_;
    const double            adjustVol_;
};

// EmployPlannerが静的関数となっているためインスタンス化しないが、意味論的に
// 要求雇用数->雇用計画策定->オファー数決定という手順を踏むほうが望ましい。
// このクラスはその統括を行う
class EmployPlanningSystem final {
  public:
    [[nodiscard]] explicit EmployPlanningSystem(RandomGenerator& masterRng) noexcept
        : offerPlanner_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        offerPlanner_.acceptMediator(mediator);
    }

    [[nodiscard]] auto plan(const HeadCount desiredEmploy, IMediator auto& mediator) noexcept
        -> HeadCount {
        const auto employPlan = EmployPlanner::plan(desiredEmploy);
        mediator.publishEmployPlan(employPlan);
        const auto offerPlan = offerPlanner_.plan(employPlan);
        return offerPlan;
    }

    void reset() noexcept { offerPlanner_.reset(); }

  private:
    OfferPlanner offerPlanner_;
};
}  // namespace abm::labor::demander::planner