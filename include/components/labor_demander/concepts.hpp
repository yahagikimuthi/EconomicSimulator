#pragma once

#include <concepts>
#include <functional>

#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor::demander {
template <typename T>
concept IPlanner = requires(T t, HeadCount headCount, world::CensusDropBox dropBox) {
    { t.judgePlan(headCount) } -> std::same_as<void>;
    { t.wagePlan() } -> std::same_as<Wage>;
    { t.offerPlan() } -> std::same_as<HeadCount>;
    { t.endStep(dropBox, headCount, headCount) } -> std::same_as<void>;
};

template <typename F>
concept AddRosterFn = requires(F f, AgentID id, Wage wage) {
    { f(id, wage) } -> std::same_as<world::RosterEntry&>;
};

template <typename T>
concept IRecruiter = requires(
    T                                                 t,
    std::function<world::RosterEntry&(AgentID, Wage)> addRoster,
    AgentID                                           agentId,
    HeadCount                                         headCount,
    tbb::concurrent_vector<world::LaborRequest>       requestBox,
    world::CensusDropBox                              dropBox
) {
    { t.post(agentId, headCount, requestBox) } -> std::same_as<void>;
    { t.offer() } -> std::same_as<void>;
    { t.registerMember(addRoster) } -> std::same_as<void>;
    { t.endStep(dropBox) } -> std::same_as<void>;
};

template <typename T>
concept ILaborDemander = requires(
    T                                           t,
    AgentID                                     id,
    HeadCount                                   headCount,
    tbb::concurrent_vector<world::LaborRequest> requestBox,
    world::Workspace                            workspace,
    world::CensusDropBox                        dropBox
) {
    { t.post(id, headCount, requestBox) } -> std::same_as<void>;
    { t.offer() } -> std::same_as<void>;
    { t.layOffs(headCount) } -> std::same_as<void>;
    { t.registerMember(workspace) } -> std::same_as<void>;
    { t.acceptResignation() } -> std::same_as<void>;
    { t.employeeCnt() } -> std::same_as<HeadCount>;
    { t.sumWage() } -> std::same_as<Money>;
    { t.endStep(dropBox) } -> std::same_as<void>;
};
}  // namespace labor::demander