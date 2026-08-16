#pragma once

#include <concepts>

#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace abm::labor::demander {
template <typename F>
concept AddRosterFn = requires(F f, AgentID id, Wage wage) {
    { f(id, wage) } -> std::same_as<RosterEntry&>;
};

struct RecruitPlan {
    const Wage      wage;
    const HeadCount offer;
};

struct RecruitResult {
    const HeadCount applicants;
    const HeadCount employ;
};
}  // namespace abm::labor::demander