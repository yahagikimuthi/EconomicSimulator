#pragma once

#include <concepts>

#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander {
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
concept IMediator = requires(T t, HeadCount employPlan) {
    { t.publishEmployPlan(employPlan) } -> std::same_as<void>;
} and requires(T t, RecruitResult& result) {
    { t.publishRecruitResult(result) } -> std::same_as<void>;
};
}  // namespace abm::labor::demander