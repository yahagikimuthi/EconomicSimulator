#pragma once

#include <concepts>
#include <optional>
#include <variant>

#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/common.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander {
using Entry   = LaborEntry;
using Request = LaborRequest;
using Market  = LaborMarket;

template <typename F>
concept AddRosterFn = requires(F f, AgentID id, Wage wage) {
    { f(id, wage) } -> std::same_as<RosterEntry&>;
};

struct RecruitPlan final {
    const Wage      wage;
    const HeadCount offer;
};

struct RecruitResult final {
    const HeadCount applicants;
    const HeadCount employ;
};

template <typename T>
class Memory final {
  public:
    [[nodiscard]] explicit constexpr Memory(const T l) noexcept : log_{l} {}

    void reset() noexcept {
        if (not next_) return;
        log_ = next_, next_.reset();
    }
    void clearLog() noexcept { log_.reset(); }
    void next(const T next) noexcept { next_ = next; }

    [[nodiscard]] auto log() const noexcept -> std::optional<T> { return log_; }

  private:
    std::optional<T> log_;
    std::optional<T> next_{std::nullopt};
};

template <typename T>
class Cache final {
  public:
    [[nodiscard]] explicit constexpr Cache(const T t) noexcept : cache_{t} {}
    [[nodiscard]] auto cache() const noexcept -> T { return cache_; }

    void next(const T next) noexcept { next_ = next; }
    void reset() noexcept {
        if (not next_) return;
        cache_ = *next_, next_.reset();
    }

  private:
    T                cache_;
    std::optional<T> next_;
};

class CentralMemory {
  public:
    [[nodiscard]] CentralMemory() noexcept = default;

    void logging(CensusDropBox& dropBox) noexcept {
        if (employPlan_) {
            ASSERT(employPlan_ >= HeadCount{0.0});
            dropBox.postedEmployments.emplace_back(employPlan_->value());
            employPlan_.reset();
        }
        if (wagePlan_) {
            ASSERT(wagePlan_ >= Wage{0.0});
            dropBox.postedWages.emplace_back(wagePlan_->value());
            wagePlan_.reset();
        }
    }

    void listenEmployPlan(const HeadCount employPlan) noexcept {
        ASSERT(employPlan >= HeadCount{0.0});
        employPlan_ = employPlan;
    }

    void listenRecruitPlan(const RecruitPlan& plan) noexcept {
        ASSERT(plan.wage >= Wage{0.0});
        wagePlan_ = plan.wage;
    }

  private:
    std::optional<HeadCount> employPlan_{std::nullopt};
    std::optional<Wage>      wagePlan_{std::nullopt};
};

template <typename T, typename U = std::monostate>
concept IMediator =
    requires(T t, U& u, HeadCount employPlan, RecruitPlan plan, RecruitResult result) {
        { t.publishEmployPlan(employPlan) } -> std::same_as<void>;
        { t.publishRecruitPlan(plan) } -> std::same_as<void>;
        { t.publishRecruitResult(result) } -> std::same_as<void>;
        { t.subscribeEmployPlan(u) } -> std::same_as<void>;
        { t.subscribeRecruitPlan(u) } -> std::same_as<void>;
        { t.subscribeRecruitResult(u) } -> std::same_as<void>;
    };
}  // namespace abm::labor::demander