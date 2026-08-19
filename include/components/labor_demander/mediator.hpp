#pragma once

#include <array>
#include <optional>
#include <variant>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/empoy_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"
#include "core/values/labor.hpp"

namespace abm::labor::demander::mediator {
using EmployPlanListener    = std::variant<planner::WagePlannerMemory>;
using RecruitResultListener = std::variant<planner::WagePlannerMemory, planner::OfferPlannerMemory>;

class Mediator final {
    template <typename T, int N>
    using OptArr = std::array<std::optional<T>, N>;

  public:
    Mediator() noexcept = default;
    void publishEmployPlan(const HeadCount employPlan) noexcept {
        for (std::optional<EmployPlanListener&> listenerOpt : employPlanListeners_) {
            if (not listenerOpt) continue;
            std::visit(
                [employPlan](auto&& listener) -> void { listener.listenEmployPlan(employPlan); },
                *listenerOpt
            );
        }
    }
    void publishRecruitResult(const RecruitResult& result) noexcept {
        for (std::optional<RecruitResultListener&> listenerOpt : recruitResultListeners_) {
            if (not listenerOpt) continue;
            std::visit(
                [&](auto&& listener) -> void { listener.listenRecruitResult(result); }, *listenerOpt
            );
        }
    }

  private:
    OptArr<EmployPlanListener&, 1>    employPlanListeners_;
    OptArr<RecruitResultListener&, 2> recruitResultListeners_;
};
}  // namespace abm::labor::demander::mediator

namespace abm::labor::demander {
using Mediator = mediator::Mediator;
}