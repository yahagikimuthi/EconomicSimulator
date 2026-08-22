#pragma once

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/employ_planner.hpp"
#include "core/setting.hpp"
#include "core/util.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"

namespace abm::base_goods::supplier {
class Producer final {
  public:
    [[nodiscard]] explicit constexpr Producer(RandomGenerator& masterRng) noexcept
        : baseProductPower{masterRng.random(setting::productPower)},
          inventory_{masterRng.random(setting::inventory)} {}

    void acceptMediator(IMediator auto& mediator) noexcept { mediator.subscribeTradeResult(*this); }
    void endStep(const GoodsQuantity unsoldAmount) noexcept {
        ASSERT(unsoldAmount >= GoodsQuantity{0.0});
        inventory_ = unsoldAmount;
        workspace_.resetInput();
    }
    [[nodiscard]] auto workspace() noexcept -> Workspace& { return workspace_; }
    [[nodiscard]] auto inventory() const noexcept -> GoodsQuantity {
        ASSERT(inventory_ >= GoodsQuantity{0.0});
        return inventory_;
    }
    [[nodiscard]] auto firmProductPower() const noexcept -> double { return baseProductPower; }

    [[nodiscard]] auto produce() noexcept -> GoodsQuantity {
        const auto workerInput = workspace_.totalInput();
        workspace_.resetInput();
        ASSERT(workerInput >= GoodsQuantity{0.0});
        const auto production = workerInput * baseProductPower;
        ASSERT(inventory_ >= GoodsQuantity{0.0});
        const auto out = production + inventory_;
        inventory_     = GoodsQuantity{0.0};

        ASSERT(out >= GoodsQuantity{0.0});
        return out;
    }

    void listenTradeResult(const TradeResult& result) noexcept {
        ASSERT(result.unsoldAmount >= GoodsQuantity{0.0});
        inventory_ += result.unsoldAmount;
    }

    void addProducingEquip(const GoodsQuantity productionGoods) noexcept {
        ASSERT(productionGoods >= GoodsQuantity{0.0});
        productionGoods_ += productionGoods;
    }

    void reset(CensusDropBox& dropBox) noexcept {
        dropBox.inventories.emplace_back(inventory_.value());
    }

  private:
    Workspace     workspace_;
    const double  baseProductPower;
    GoodsQuantity inventory_;
    GoodsQuantity productionGoods_{0.0};
};

class ProducingSystem final {
  public:
    [[nodiscard]] explicit constexpr ProducingSystem(RandomGenerator& masterRng) noexcept
        : employPlanner_{masterRng}, producer_{masterRng} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        employPlanner_.acceptMediator(mediator);
        producer_.acceptMediator(mediator);
    }

    [[nodiscard]] auto calcDesiredEmploy(
        const GoodsQuantity requiresSupply, const HeadCount employee
    ) noexcept -> HeadCount {
        const auto inventory    = producer_.inventory();
        const auto productPower = producer_.firmProductPower();
        return employPlanner_.plan(productPower, employee, requiresSupply - inventory);
    }

    void addProducingEquip(const GoodsQuantity productionGoods) noexcept {
        producer_.addProducingEquip(productionGoods);
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return producer_.workspace(); }

    [[nodiscard]] auto produce() noexcept -> GoodsQuantity { return producer_.produce(); }

    void reset(CensusDropBox& dropBox) noexcept {
        producer_.reset(dropBox);
        employPlanner_.reset();
    }

  private:
    EmployPlanner employPlanner_;
    Producer      producer_;
};
}  // namespace abm::base_goods::supplier