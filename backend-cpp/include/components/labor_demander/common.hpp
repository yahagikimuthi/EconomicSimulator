#pragma once

#include <cassert>
#include <concepts>
#include <optional>
#include <variant>

#include "values/common.hpp"
#include "values/labor.hpp"
#include "world/drop_box.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander {
template <typename F>
concept AddRosterFn = requires(F f, AgentID id, Wage wage) {
    { f(id, wage) } -> std::same_as<RosterEntry&>;
};

template <typename F>
concept TryPayWageFn = requires(F f, Wage wage) {
    { f(wage) } -> std::same_as<Money>;
};

struct RecruitPlan final {
    const Wage      wage;
    const HeadCount employ;
    const HeadCount offer;
};

struct RecruitResult final {
    const HeadCount applicants;
    const HeadCount employ;
};

template <typename T>
class Memory final {
  public:
    explicit Memory(const T log) noexcept : log_{log} {}

    [[nodiscard]] auto wasSetNext() const noexcept -> bool { return next_.has_value(); }
    [[nodiscard]] auto log() const noexcept -> std::optional<T> { return log_; }

    void reset() noexcept {
        if (not next_) return;
        log_ = next_;
        next_.reset();
    }
    void clearLog() noexcept { log_.reset(); }
    void next(const T next) noexcept { next_ = next; }

  private:
    std::optional<T> log_;
    std::optional<T> next_{std::nullopt};
};

class CentralMemory final {
  public:
    explicit CentralMemory() noexcept = default;

    void logging(LaborDropBox& dropBox) noexcept {
        if (employPlan_) {
            assert(employPlan_->isPositive());
            dropBox.postedEmployments.add(*employPlan_);
            employPlan_.reset();
        }
        if (wagePlan_) {
            assert(wagePlan_->isPositive());
            dropBox.postedWages.add(*wagePlan_);
            wagePlan_.reset();
        }
    }

    void listenRecruitPlan(const RecruitPlan& plan) noexcept {
        assert(plan.wage.isZeroOrMore());
        if (plan.wage.isPositive()) wagePlan_ = plan.wage;

        assert(plan.employ.isZeroOrMore());
        if (plan.employ.isPositive()) employPlan_ = plan.employ;
    }

  private:
    std::optional<HeadCount> employPlan_{std::nullopt};
    std::optional<Wage>      wagePlan_{std::nullopt};
};

template <typename T, typename U = std::monostate>
concept IMediator =
    requires(T t, U& u, HeadCount employPlan, RecruitPlan plan, RecruitResult result) {
        { t.publishRecruitPlan(plan) } -> std::same_as<void>;
        { t.publishRecruitResult(result) } -> std::same_as<void>;
        { t.subscribeRecruitPlan(u) } -> std::same_as<void>;
        { t.subscribeRecruitResult(u) } -> std::same_as<void>;
    };
}  // namespace abm::labor::demander