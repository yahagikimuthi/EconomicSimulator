#pragma once

#include <algorithm>
#include <optional>

#include "components/base_goods_supplier/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/goods.hpp"
#include "values/labor.hpp"
#include "values/math.hpp"

namespace abm::base_goods::supplier {
// 前回の取引計画中、供給量が必要
class EmployPlannerMemory final {
  public:
    explicit EmployPlannerMemory(RandomGenerator& masterRng) noexcept
        : supply_{GoodsQuantity{masterRng.random(setting::lastSupply)}} {}
    [[nodiscard]] auto lastSupply() const noexcept -> std::optional<GoodsQuantity> {
        return supply_.log();
    }
    void clearLog() noexcept { supply_.clearLog(); }
    void reset() noexcept { supply_.reset(); }
    void listenTradePlan(const TradePlan& plan) noexcept {
        ASSERT(plan.supply >= GoodsQuantity{0.0});
        supply_.next(plan.supply);
    }

  private:
    Memory<GoodsQuantity> supply_;
};

class EmployPlanner final {
  public:
    explicit EmployPlanner(RandomGenerator& masterRng) noexcept
        : memory_{masterRng}, cache_{HeadCount{masterRng.random(setting::desiredEmploy)}} {}

    void acceptMediator(IMediator auto& mediator) noexcept { mediator.subscribeTradePlan(memory_); }

    [[nodiscard]] auto plan(
        const double firmProductPower, const HeadCount employee, const GoodsQuantity targetCapital
    ) noexcept -> HeadCount {
        ASSERT(firmProductPower >= 0.0);
        ASSERT(employee >= HeadCount{0.0});

        const auto out = calc(firmProductPower, employee, targetCapital);
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
    [[nodiscard]] auto calc(
        const double firmProductPower, const HeadCount employee, const GoodsQuantity targetCapital
    ) noexcept -> std::optional<HeadCount> {
        ASSERT(firmProductPower >= 0.0);
        ASSERT(employee >= HeadCount{0.0});

        const auto lastSupply = memory_.lastSupply();
        if (not lastSupply) return std::nullopt;
        ASSERT(*lastSupply >= GoodsQuantity{0.0});
        const auto isEmploying = employee != HeadCount{0.0};
        const auto avgPower =
            isEmploying ? lastSupply->value() / employee.value() : firmProductPower;
        const auto out     = (avgPower != 0.0) ? targetCapital.value() / avgPower : 1.0;
        const auto guarded = std::min(out, static_cast<double>(::abm::setting::agent_count::hhold));
        return ceil(HeadCount{guarded});
    }

    EmployPlannerMemory memory_;
    Cache<HeadCount>    cache_;
};
}  // namespace abm::base_goods::supplier