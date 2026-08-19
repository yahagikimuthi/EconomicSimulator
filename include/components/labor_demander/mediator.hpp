#pragma once

#include <array>
#include <variant>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/empoy_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"
#include "core/values/labor.hpp"

namespace abm::labor::demander::mediator {
using EmployPlanListener = std::variant<planner::WagePlannerMemory*>;
using RecruitResultListener =
    std::variant<planner::WagePlannerMemory*, planner::OfferPlannerMemory*>;

class Mediator final {
    friend class ::abm::labor::demander::Factory;

  public:
    Mediator() noexcept = default;
    void publishEmployPlan(const HeadCount employPlan) noexcept {
        for (auto ptr : employPlanListeners_) {
            std::visit(
                [employPlan](auto&& listener) -> void {
                    if (listener == nullptr) return;
                    listener->listenEmployPlan(employPlan);
                },
                ptr
            );
        }
    }
    void publishRecruitResult(const RecruitResult& result) noexcept {
        for (auto ptr : recruitResultListeners_) {
            std::visit(
                [&](auto&& listener) -> void {
                    if (listener == nullptr) return;
                    listener->listenRecruitResult(result);
                },
                ptr
            );
        }
    }

  private:
    std::array<EmployPlanListener, 1>    employPlanListeners_;
    std::array<RecruitResultListener, 2> recruitResultListeners_;
};
}  // namespace abm::labor::demander::mediator

namespace abm::labor::demander {
using Mediator = mediator::Mediator;
}