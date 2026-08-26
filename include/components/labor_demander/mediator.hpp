#pragma once

#include <optional>
#include <tuple>
#include <type_traits>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/empoy_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"
#include "core/values/labor.hpp"

namespace abm::labor::demander {
template <typename... Ts>
class Listener {
    static_assert(sizeof...(Ts) >= 1UZ);

  public:
    [[nodiscard]] Listener() noexcept = default;

    template <typename T>
        requires std::disjunction_v<std::is_same<T, Ts>...>
    void add(T& t) noexcept {
        std::get<std::optional<T&>>(listeners_) = t;
    }

    template <typename F>
        requires std::conjunction_v<std::is_invocable<F, Ts>...>
    void notice(F methodCaller) noexcept {
        auto call = [&](auto&& opt) -> void {
            if (opt) {
                methodCaller(*opt);
            }
        };
        std::apply([&](auto&&... opts) noexcept -> void { ((call(opts)), ...); }, listeners_);
    }

  private:
    std::tuple<std::optional<Ts&>...> listeners_;
};

class Mediator final {
    using EmployPlanListener =
        Listener<planner::WagePlannerMemory, planner::OfferPlannerMemory, CentralMemory>;
    using RecruitPlanListener   = Listener<CentralMemory>;
    using RecruitResultListener = Listener<planner::WagePlannerMemory, planner::OfferPlannerMemory>;

  public:
    [[nodiscard]] constexpr Mediator() noexcept = default;

    template <typename T>
    void subscribeEmployPlan(T& t) noexcept {
        employPlanListeners_.add(t);
    }

    template <typename T>
    void subscribeRecruitPlan(T& t) noexcept {
        recruitPlanListeners_.add(t);
    }

    template <typename T>
    void subscribeRecruitResult(T& t) noexcept {
        recruitResultListeners_.add(t);
    }

    void publishEmployPlan(const HeadCount employPlan) noexcept {
        employPlanListeners_.notice([employPlan](auto&& listener) noexcept -> void {
            listener.listenEmployPlan(employPlan);
        });
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
    EmployPlanListener    employPlanListeners_;
    RecruitPlanListener   recruitPlanListeners_;
    RecruitResultListener recruitResultListeners_;
};
}  // namespace abm::labor::demander