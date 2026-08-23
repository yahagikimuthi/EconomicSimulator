#pragma once

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/employ_planner.hpp"
#include "core/setting.hpp"
#include "core/util.hpp"
#include "core/values/goods.hpp"
#include "core/values/integrate.hpp"
#include "core/values/labor.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"

namespace abm::base_goods::supplier {
class ProducingSystem final {
  public:
    [[nodiscard]] explicit constexpr ProducingSystem(RandomGenerator& masterRng) noexcept
        : employPlanner_{masterRng},
          baseProductPower_{masterRng.random(setting::productPower)},
          producerGoodsEfficiency_{masterRng.random(setting::producerGoodsEfficiency)},
          inventory_{masterRng.random(setting::inventory)} {}

    void acceptMediator(IMediator auto& mediator) noexcept {
        employPlanner_.acceptMediator(mediator);
    }

    [[nodiscard]] auto calcDesiredEmploy(
        const GoodsQuantity requiresSupply, const HeadCount employee
    ) noexcept -> HeadCount {
        return employPlanner_.plan(baseProductPower_, employee, requiresSupply - inventory_);
    }

    [[nodiscard]] auto calcDesiredProductionGoods(const GoodsQuantity requiresSupply
    ) const noexcept -> GoodsQuantity {
        return (requiresSupply / (baseProductPower_ * producerGoodsEfficiency_)) - productionGoods_;
    }

    void addProducingEquip(const GoodsQuantity productionGoods) noexcept {
        ASSERT(productionGoods >= GoodsQuantity{0.0});
        productionGoods_ += productionGoods;
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return workspace_; }

    [[nodiscard]] auto produce() noexcept -> GoodsQuantity {
        const auto workerInput          = workspace_.totalInput();
        const auto productionGoodsInput = productionGoods_ * producerGoodsEfficiency_;
        workspace_.resetInput();
        ASSERT(workerInput >= GoodsQuantity{0.0});
        ASSERT(productionGoodsInput >= GoodsQuantity{0.0});
        const auto production = baseProductPower_ * min(workerInput, productionGoodsInput);
        ASSERT(inventory_ >= GoodsQuantity{0.0});
        const auto out = production + inventory_;
        inventory_     = GoodsQuantity{0.0};

        ASSERT(out >= GoodsQuantity{0.0});
        return out;
    }

    void listenTradeResult(const TradeResult& result) noexcept {
        inventory_ += result.unsoldAmount;
    }

    void reset(CensusDropBox& dropBox) noexcept {
        dropBox.inventories.emplace_back(inventory_.value());
        employPlanner_.reset();
    }

  private:
    EmployPlanner employPlanner_;
    Workspace     workspace_;
    const double  baseProductPower_;
    const double  producerGoodsEfficiency_;
    GoodsQuantity inventory_;
    GoodsQuantity productionGoods_{0.0};
};
}  // namespace abm::base_goods::supplier