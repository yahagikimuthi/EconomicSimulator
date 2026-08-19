#pragma once

#include <concepts>
#include <optional>

#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander {
using Entry   = LaborEntry;
using Request = LaborRequest;

class LaborDemanderFactory;

template <typename F>
concept AddRosterFn = requires(F f, AgentID id, Wage wage) {
    { f(id, wage) } -> std::same_as<RosterEntry&>;
};

struct RecruitPlan final {
    Wage      wage;
    HeadCount offer;
};

struct RecruitResult final {
    const HeadCount applicants;
    const HeadCount employ;
};

template <typename T>
class Memory final {
  public:
    [[nodiscard]] explicit Memory(const T l) noexcept : log{l} {}

    void reset() noexcept {
        if (not next) return;
        log = next, next.reset();
    }
    void clearLog() noexcept { log.reset(); }

    std::optional<T> log;
    std::optional<T> next{std::nullopt};
};

template <typename T>
class Cache final {
  public:
    [[nodiscard]] explicit Cache(const T t) noexcept;
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

template <typename T>
concept IMediator = requires(T t, HeadCount employPlan) {
    { t.publishEmployPlan(employPlan) } -> std::same_as<void>;
} and requires(T t, RecruitResult& result) {
    { t.publishRecruitResult(result) } -> std::same_as<void>;
};
}  // namespace abm::labor::demander