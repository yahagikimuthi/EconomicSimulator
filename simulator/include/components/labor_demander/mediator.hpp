#pragma once

#include "components/common.hpp"
#include "components/labor_demander/common.hpp"
#include "components/labor_demander/offer_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"

namespace abm::labor::demander {
class Mediator final {
    using PlanListener =
        Listener<CentralMemory, planner::OfferPlannerMemory, planner::WagePlannerMemory>;
    using ResultListener = Listener<planner::OfferPlannerMemory, planner::WagePlannerMemory>;

  public:
    explicit Mediator() noexcept = default;

    void subscribeRecruitPlan(IsListenerOrMono<PlanListener> auto& listener) noexcept {
        recruitPlanListeners_.add(listener);
    }

    void subscribeRecruitResult(IsListenerOrMono<ResultListener> auto& listener) noexcept {
        recruitResultListeners_.add(listener);
    }

    void publishRecruitPlan(const RecruitPlan& plan) noexcept {
        recruitPlanListeners_.notify([&](auto& listener) noexcept -> void {
            listener.listenRecruitPlan(plan);
        });
    }

    void publishRecruitResult(const RecruitResult& result) noexcept {
        recruitResultListeners_.notify([&](auto& listener) noexcept -> void {
            listener.listenRecruitResult(result);
        });
    }

  private:
    PlanListener   recruitPlanListeners_;
    ResultListener recruitResultListeners_;
};
}  // namespace abm::labor::demander