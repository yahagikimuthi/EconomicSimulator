#pragma once

#include <optional>

#include "components/base_goods_supplier/common.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"
#include "setting.hpp"
#include "util.hpp"

namespace abm::base_goods::supplier {
// 前回の取引計画中、供給量が必要
class EmployPlannerMemory final {
  public:
    [[nodiscard]] explicit EmployPlannerMemory(RandomGenerator& masterRng) noexcept
        : supply_{GoodsQuantity{masterRng.random(setting::lastSupply)}} {}
    [[nodiscard]] auto lastSupply() const noexcept -> std::optional<GoodsQuantity> {
        return supply_.log;
    }
    void clearLog() noexcept { supply_.clearLog(); }
    void reset() noexcept { supply_.reset(); }
    void listenTradePlan(const TradePlan& plan) noexcept { supply_.next = plan.supply; }

  private:
    Memory<GoodsQuantity> supply_;
};

class EmployPlanner final {
  public:
    [[nodiscard]] explicit EmployPlanner(RandomGenerator& masterRng) noexcept
        : memory_{masterRng}, cache_{HeadCount{masterRng.random(setting::desiredEmploy)}} {}

    void acceptMediator(IMediator auto& mediator) noexcept { mediator.subscribeTradePlan(memory_); }

    [[nodiscard]] auto plan(
        const double        firmProductPower,
        const HeadCount     employee,
        const GoodsQuantity targetProduction
    ) noexcept -> HeadCount {
        const auto out = calc(firmProductPower, employee, targetProduction);
        memory_.clearLog();
        if (not out) return cache_.cache();
        cache_.next(*out);
        return *out;
    }

    void reset() noexcept {
        cache_.reset();
        memory_.reset();
    }

  private:
    //! 目標在庫率のこと考えてないね
    [[nodiscard]] auto calc(
        const double        firmProductPower,
        const HeadCount     employee,
        const GoodsQuantity targetProduction
    ) noexcept -> std::optional<HeadCount> {
        ASSERT(firmProductPower >= 0.0);
        ASSERT(employee >= HeadCount{0.0});
        ASSERT(targetProduction >= GoodsQuantity{0.0});

        const auto lastSupply = memory_.lastSupply();
        if (not lastSupply) return std::nullopt;
        ASSERT(*lastSupply >= GoodsQuantity{0.0});
        const auto isEmploying = employee == HeadCount{0.0};
        const auto avgPower =
            isEmploying ? lastSupply->value() / employee.value() : firmProductPower;
        const auto out = avgPower != 0.0 ? targetProduction.value() / avgPower : 1.0;
        return HeadCount{out}.ceil();
    }

    EmployPlannerMemory memory_;
    Cache<HeadCount>    cache_;
};
}  // namespace abm::base_goods::supplier