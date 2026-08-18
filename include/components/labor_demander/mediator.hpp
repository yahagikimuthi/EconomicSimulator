#pragma once

#include <array>
#include <variant>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/empoy_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"
#include "core/values/labor.hpp"

namespace abm::labor::demander::mediator {
using EmployPlanListener    = std::variant<planner::WagePlannerMemory>;
using RecruitResultListener = std::variant<planner::WagePlannerMemory, planner::OfferPlannerMemory>;

class Mediator final {
  public:
    Mediator() = default;
    void publishEmployPlan(const HeadCount employPlan) {
        for (std::optional<EmployPlanListener&> listenerOpt : employPlanListeners_) {
            if (not listenerOpt) return;
            std::visit(
                [employPlan](auto&& listener) -> void { listener.listenEmployPlan(employPlan); },
                *listenerOpt
            );
        }
    }
    void publishRecruitResult(const RecruitResult& result) {
        for (std::optional<RecruitResultListener&> listenerOpt : recruitResultListeners_) {
            if (not listenerOpt) return;

            std::visit(
                [&](auto&& listener) -> void { listener.listenRecruitResult(result); }, *listenerOpt
            );
        }
    }

  private:
    std::array<std::optional<EmployPlanListener&>, 1>    employPlanListeners_;
    std::array<std::optional<RecruitResultListener&>, 2> recruitResultListeners_;
};
}  // namespace abm::labor::demander::mediator

namespace abm::labor::demander {
using Mediator = mediator::Mediator;
}