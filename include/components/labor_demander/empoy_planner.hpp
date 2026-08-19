#pragma once

#include <optional>

#include "components/labor_demander/common.hpp"
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
        : applicants_{HeadCount{masterRng.rand(10, 20)}},
          employPlan_{HeadCount{masterRng.rand(10, 20)}} {}
    [[nodiscard]] auto rememberLastApplicants() const noexcept -> std::optional<HeadCount> {
        return applicants_.log;
    }
    [[nodiscard]] auto rememberLastEmployPlan() const noexcept -> std::optional<HeadCount> {
        return employPlan_.log;
    }
    void clearLog() noexcept { applicants_.clearLog(), employPlan_.clearLog(); }
    void commit() noexcept { applicants_.commit(), employPlan_.commit(); }
    void listenRecruitResult(const RecruitResult& result) noexcept {
        applicants_.next = result.applicants;
    }

  private:
    Memory<HeadCount> applicants_;
    Memory<HeadCount> employPlan_;
};

class OfferPlanner final {
  public:
    [[nodiscard]] explicit OfferPlanner(RandomGenerator& rng) noexcept
        : memory_{rng},
          cache_{rng.rand(0.1, 0.2)},
          rng_{pcg32{rng.makeUint64(), rng.makeUint64()}},
          adjustVol_{rng.rand(0.1, 0.2)} {}

    [[nodiscard]] auto plan(const HeadCount employPlan) noexcept -> HeadCount {
        return HeadCount{employPlan * (1.0 + planOfferRate())};
    }

    void commit() noexcept {
        memory_.commit();
        cache_.commit();
    }

  private:
    [[nodiscard]] auto planOfferRate() noexcept -> double {
        const auto nextRate = calcOfferRate();
        memory_.clearLog();
        if (not nextRate) return cache_.cache();
        cache_.next(*nextRate);
        return *nextRate;
    }

    [[nodiscard]] auto calcOfferRate() const noexcept -> std::optional<double> {
        const auto lastApplicants = memory_.rememberLastApplicants();
        const auto lastEmployPlan = memory_.rememberLastEmployPlan();
        if (not lastApplicants or not lastEmployPlan) return std::nullopt;
        const auto alpha       = rng_.randNormal(0.0, adjustVol_);
        const auto shouldRaise = *lastApplicants < *lastEmployPlan;
        const auto next        = cache_.cache() + (shouldRaise ? alpha : -alpha);
        return std::max(0.0, next);
    }

    OfferPlannerMemory      memory_;
    Cache<double>           cache_;
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

    [[nodiscard]] auto plan(const HeadCount desiredEmploy, IMediator auto& mediator) noexcept
        -> HeadCount {
        const auto employPlan = EmployPlanner::plan(desiredEmploy);
        mediator.publishEmployPlan(employPlan);
        const auto offerPlan = offerPlanner_.plan(employPlan);
        return offerPlan;
    }

    void commit() noexcept { offerPlanner_.commit(); }

  private:
    OfferPlanner offerPlanner_;
};
}  // namespace abm::labor::demander::planner