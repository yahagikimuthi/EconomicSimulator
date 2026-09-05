#pragma once

#include "components/common.hpp"
#include "components/labor_demander/common.hpp"
#include "components/labor_demander/offer_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"

namespace abm::labor::demander {
class Mediator final {
    using RecruitPlanListener =
        Listener<planner::WagePlannerMemory, planner::OfferPlannerMemory, CentralMemory>;
    using RecruitResultListener = Listener<planner::WagePlannerMemory, planner::OfferPlannerMemory>;

  public:
    explicit Mediator() noexcept = default;

    template <typename T>
    void subscribeRecruitPlan(T& t) noexcept {
        recruitPlanListeners_.add(t);
    }

    template <typename T>
    void subscribeRecruitResult(T& t) noexcept {
        recruitResultListeners_.add(t);
    }

    void publishRecruitPlan(const RecruitPlan& plan) noexcept {
        recruitPlanListeners_.notice([&](auto&& listener) noexcept -> void {
            listener.listenRecruitPlan(plan);
        });
    }

    void publishRecruitResult(const RecruitResult& result) noexcept {
        recruitResultListeners_.notice([&](auto&& listener) noexcept -> void {
            listener.listenRecruitResult(result);
        });
    }

  private:
    RecruitPlanListener   recruitPlanListeners_;
    RecruitResultListener recruitResultListeners_;
};
}  // namespace abm::labor::demander