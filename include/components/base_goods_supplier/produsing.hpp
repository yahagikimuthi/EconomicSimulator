#pragma once

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/employ_planner.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"

namespace abm::base_goods::supplier {
class Producer final {
  public:
    [[nodiscard]] explicit Producer(RandomGenerator& masterRng) noexcept;

    void acceptMediator(IMediator auto& mediator) noexcept { mediator.subscribeTradePlan(*this); }
    void endStep(const GoodsQuantity unsoldAmount, CensusDropBox& dropBox) noexcept {
        inventory_ = unsoldAmount;
        dropBox.inventories.emplace_back(inventory_.value());
        workspace_.resetInput();
    }
    [[nodiscard]] auto workspace() noexcept -> Workspace& { return workspace_; }
    [[nodiscard]] auto inventory() const noexcept -> GoodsQuantity { return inventory_; }
    [[nodiscard]] auto firmProductPower() const noexcept -> double { return firmProductPower_; }

    [[nodiscard]] auto produce() noexcept -> GoodsQuantity {
        const auto workerInput = workspace_.totalInput();
        const auto production  = workerInput * firmProductPower_;
        const auto out         = production + inventory_;
        inventory_             = GoodsQuantity{0.0};
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

class ProducingSystem final {
  public:
    [[nodiscard]] explicit ProducingSystem(RandomGenerator& masterRng) noexcept
        : employPlanner_{masterRng}, producer_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        employPlanner_.acceptMediator(mediator);
        producer_.acceptMediator(mediator);
    }

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