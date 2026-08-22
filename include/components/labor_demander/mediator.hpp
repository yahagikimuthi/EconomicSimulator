#pragma once

#include <array>
#include <concepts>
#include <optional>
#include <variant>

#include "components/labor_demander/common.hpp"
#include "components/labor_demander/empoy_planner.hpp"
#include "components/labor_demander/wage_planner.hpp"
#include "core/values/labor.hpp"

namespace abm::labor::demander {
class Mediator final {
    template <typename T>
    using Opt                = std::optional<T>;
    using EmployPlanListener = std::variant<
        Opt<planner::WagePlannerMemory&>,
        Opt<planner::OfferPlannerMemory&>,
        Opt<CentralMemory&>>;
    using RecruitPlanListener = std::variant<Opt<CentralMemory&>>;
    using RecruitResultListener =
        std::variant<Opt<planner::WagePlannerMemory&>, Opt<planner::OfferPlannerMemory&>>;

  public:
    [[nodiscard]] constexpr Mediator() noexcept = default;

    template <typename T>
    void subscribeEmployPlan(T& t) noexcept {
        auto& arr = employPlanListeners_;
        if constexpr (std::is_same_v<T, planner::WagePlannerMemory>) {
            arr[0] = t;
        } else if constexpr (std::is_same_v<T, planner::OfferPlannerMemory>) {
            arr[1] = t;
        } else if constexpr (std::is_same_v<T, CentralMemory>) {
            arr[2] = t;
        } else {
            static_assert(false);
        }
    }

    template <typename T>
    void subscribeRecruitPlan(T& t) noexcept {
        auto& arr = recruitPlanListeners_;
        if constexpr (std::same_as<T, CentralMemory>) {
            arr[0] = t;
        } else {
            static_assert(false);
        }
    }

    template <typename T>
    void subscribeRecruitResult(T& t) noexcept {
        auto& arr = recruitResultListeners_;
        if constexpr (std::is_same_v<T, planner::WagePlannerMemory>) {
            arr[0] = t;
        } else if constexpr (std::is_same_v<T, planner::OfferPlannerMemory>) {
            arr[1] = t;
        } else {
            static_assert(false);
        }
    }

    void publishEmployPlan(const HeadCount employPlan) noexcept {
        for (auto opt : employPlanListeners_) {
            std::visit(
                [employPlan](auto&& listener) -> void {
                    if (not listener) return;
                    listener->listenEmployPlan(employPlan);
                },
                opt
            );
        }
    }

    void publishRecruitPlan(const RecruitPlan& plan) noexcept {
        for (auto opt : recruitPlanListeners_) {
            std::visit(
                [&](auto&& listener) -> void {
                    if (not listener) return;
                    listener->listenRecruitPlan(plan);
                },
                opt
            );
        }
    }

    void publishRecruitResult(const RecruitResult& result) noexcept {
        for (auto opt : recruitResultListeners_) {
            std::visit(
                [&](auto&& listener) -> void {
                    if (not listener) return;
                    listener->listenRecruitResult(result);
                },
                opt
            );
        }
    }

  private:
    std::array<EmployPlanListener, 3>    employPlanListeners_;
    std::array<RecruitPlanListener, 1>   recruitPlanListeners_;
    std::array<RecruitResultListener, 2> recruitResultListeners_;
};
}  // namespace abm::labor::demander