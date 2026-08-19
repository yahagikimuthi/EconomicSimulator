#pragma once

#include <algorithm>
#include <cmath>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/employ_planner.hpp"
#include "config.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"

namespace abm::base_goods::supplier {
class Producer final {
  public:
    [[nodiscard]] explicit Producer(
        RandomGenerator& masterRng, const Workspace& workspace
    ) noexcept;

    [[nodiscard]] auto product() const noexcept -> GoodsQuantity {
        return workspace_.totalInput() + inventory_;
    }

    [[nodiscard]] auto calcDesiredEmploy(
        const GoodsQuantity targetSupply,
        const GoodsQuantity lastSupply,
        const HeadCount     employeeCnt,
        const double        laborSupplierCnt = config::agent_count::hhold
    ) const noexcept -> HeadCount PRE(targetSupply >= GoodsQuantity{0.0})
                         PRE(lastSupply >= GoodsQuantity{0.0}) PRE(employeeCnt >= HeadCount{0.0}) {
        const auto targetProduction = GoodsQuantity{targetSupply - inventory_};
        const auto avgProductPower  = (employeeCnt != HeadCount{0.0})
                                          ? lastSupply.value() / employeeCnt.value()
                                          : firmProductPower_;
        const auto desiredEmploy =
            HeadCount{(avgProductPower != 0.0) ? targetProduction.value() / avgProductPower : 1.0};
        return HeadCount{std::min(laborSupplierCnt, std::ceil(desiredEmploy.value()))};
    }

    void endStep(const GoodsQuantity unsoldAmount, CensusDropBox& dropBox) noexcept {
        inventory_ = unsoldAmount;
        dropBox.inventories.emplace_back(inventory_.value());
        workspace_.resetInput();
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return workspace_; }

    [[nodiscard]] auto inventory() const noexcept -> GoodsQuantity { return inventory_; }

    [[nodiscard]] auto firmProductPower() const noexcept -> double { return firmProductPower_; }

    [[nodiscard]] auto produce() noexcept -> GoodsQuantity {
        const auto out = workspace_.totalInput() + inventory_;
        inventory_     = GoodsQuantity{0.0};
        return out;
    }

    void listenTradeResult(const TradeResult& result) noexcept {
        inventory_ += result.unsoldAmount;
    }

  private:
    Workspace     workspace_;
    const double  firmProductPower_;
    GoodsQuantity inventory_;
};

class ProducingSystem {
  public:
    [[nodiscard]] explicit ProducingSystem(
        RandomGenerator& masterRng, const Workspace& workspace
    ) noexcept
        : employPlanner_{masterRng}, producer_{masterRng, workspace} {}

    [[nodiscard]] auto calcDesiredEmploy(const HeadCount employee) noexcept -> HeadCount {
        const auto inventory    = producer_.inventory();
        const auto productPower = producer_.firmProductPower();
        return employPlanner_.plan(productPower, employee, inventory);
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return producer_.workspace(); }

    [[nodiscard]] auto produce() noexcept -> GoodsQuantity { return producer_.produce(); }

  private:
    EmployPlanner employPlanner_;
    Producer      producer_;
};
}  // namespace abm::base_goods::supplier