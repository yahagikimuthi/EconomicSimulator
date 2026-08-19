#pragma once

#include <algorithm>
#include <optional>

#include "components/base_goods_supplier/common.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"

namespace abm::base_goods::supplier {

// 前回の取引結果中、需要量が必要
class DemandForecastManagerMemory final {
  public:
    [[nodiscard]] DemandForecastManagerMemory();

    [[nodiscard]] auto rememberLastTotalDemand() const noexcept -> std::optional<GoodsQuantity> {
        return totalDemand_.log;
    }
    void clearLog() noexcept { totalDemand_.clearLog(); }
    void commit() noexcept { totalDemand_.commit(); }
    void listenTradeResult(const TradeResult& result) noexcept {
        totalDemand_.current = result.totalDemand;
    }

  private:
    Memory<GoodsQuantity> totalDemand_;
};

class DemandForecastManager final {
  public:
    [[nodiscard]] explicit DemandForecastManager(RandomGenerator& masterRng) noexcept;

    [[nodiscard]] auto plan() noexcept -> GoodsQuantity {
        const auto next = calcNext();
        memory_.clearLog();
        if (not next) return cache_.cache();
        cache_.memorize(*next);
        return *next;
    }

  private:
    [[nodiscard]] auto calcNext() const noexcept -> std::optional<GoodsQuantity> {
        const auto lastTotalDemand = memory_.rememberLastTotalDemand();
        if (not lastTotalDemand) return std::nullopt;
        const auto lastForecast = cache_.cache();
        const auto out          = lastForecast + (adjustment_ * (*lastTotalDemand - lastForecast));
        return guard(out);
    }

    [[nodiscard]] static auto guard(const GoodsQuantity expect) noexcept -> GoodsQuantity {
        return GoodsQuantity{std::max(expect.value(), 0.0)};
    }

    DemandForecastManagerMemory memory_;
    Cache<GoodsQuantity>        cache_;
    const double                adjustment_;
};

// 前回の取引計画中、供給量が必要
class EmployPlannerMemory final {
  public:
    [[nodiscard]] explicit EmployPlannerMemory(RandomGenerator& masterRng) noexcept;
    [[nodiscard]] auto rememberLastSupply() const noexcept -> std::optional<GoodsQuantity> {
        return supply_.log;
    }
    void clearLog() noexcept { supply_.clearLog(); }
    void commit() { supply_.commit(); }
    void listenTradePlan(const TradePlan& plan) noexcept { supply_.current = plan.supply; }

  private:
    Memory<GoodsQuantity> supply_;
};

class EmployPlanner final {
  public:
    [[nodiscard]] explicit EmployPlanner(RandomGenerator& masterRng) noexcept;

    [[nodiscard]] auto plan(
        const double firmProductPower, const HeadCount employee, const GoodsQuantity inventory
    ) noexcept -> HeadCount {
        const auto targetSupply = demandForecastManager_.plan();
        const auto out          = calc(firmProductPower, employee, targetSupply - inventory);
        memory_.clearLog();
        if (not out) return cache_.cache();
        cache_.memorize(*out);
        return *out;
    }

    void commit() noexcept {
        cache_.commit();
        memory_.commit();
    }

  private:
    [[nodiscard]] auto calc(
        const double        firmProductPower,
        const HeadCount     employee,
        const GoodsQuantity targetProduction
    ) noexcept -> std::optional<HeadCount> {
        ASSERT(firmProductPower >= 0.0);
        ASSERT(employee >= HeadCount{0.0});
        ASSERT(targetProduction >= GoodsQuantity{0.0});

        const auto lastSupply = memory_.rememberLastSupply();
        if (not lastSupply) return std::nullopt;
        ASSERT(*lastSupply >= GoodsQuantity{0.0});
        const auto isEmploying = employee == HeadCount{0.0};
        const auto avgPower =
            isEmploying ? lastSupply->value() / employee.value() : firmProductPower;
        const auto out = avgPower != 0.0 ? targetProduction.value() / avgPower : 1.0;
        return HeadCount{std::ceil(out)};
    }

    DemandForecastManager demandForecastManager_;
    EmployPlannerMemory   memory_;
    Cache<HeadCount>      cache_;
};
}  // namespace abm::base_goods::supplier