#pragma once

#include <optional>
#include <tuple>
#include <type_traits>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/empoy_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"

namespace abm::labor::demander {

template <typename... Ts>
    requires(sizeof...(Ts) > 0UZ)
class Listener final {
  public:
    explicit Listener() noexcept = default;

    template <typename T>
        requires(std::is_same_v<T, Ts> or ...)
    void add(T& t) noexcept {
        ASSERT(not std::get<std::optional<T&>>(listeners_));
        std::get<std::optional<T&>>(listeners_) = t;
    }

    template <typename F>
        requires(std::is_invocable_v<F, Ts> and ...)
    void notice(F&& methodCaller) noexcept {
        template for (auto& opt : listeners_) {
            if (opt) {
                methodCaller(*opt);
            }
        }
    }

  private:
    std::tuple<std::optional<Ts&>...> listeners_;
};

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