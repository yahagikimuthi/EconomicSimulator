#pragma once

#include <array>
#include <functional>
#include <variant>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/empoy_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"
#include "core/values/labor.hpp"

namespace abm::labor::demander::mediator {
using EmployPlanListener    = std::variant<planner::WagePlannerMemory>;
using RecruitResultListener = std::variant<planner::WagePlannerMemory, planner::OfferPlannerMemory>;

class Mediator {
    template <typename T>
    using RefWrap = std::reference_wrapper<T>;

  public:
    Mediator();
    void publishEmployPlan(const HeadCount employPlan) {
        for (RefWrap<EmployPlanListener> listenerRef : employPlanListeners_) {
            std::visit(
                [employPlan](auto&& listener) -> void { listener.listenEmployPlan(employPlan); },
                listenerRef.get()
            );
        }
    }
    void publishRecruitResult(const RecruitResult& result) {
        for (RefWrap<RecruitResultListener> listenerRef : recruitResultListeners_) {
            std::visit(
                [&](auto&& listener) -> void { listener.listenRecruitResult(result); },
                listenerRef.get()
            );
        }
    }

  private:
    std::array<RefWrap<EmployPlanListener>, 1>    employPlanListeners_;
    std::array<RefWrap<RecruitResultListener>, 2> recruitResultListeners_;
};
}  // namespace abm::labor::demander::mediator

namespace abm::labor::demander {
using Mediator = mediator::Mediator;
}