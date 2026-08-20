#pragma once

#include <tbb/concurrent_vector.h>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/mediator.hpp"
#include "components/base_goods_supplier/produsing.hpp"
#include "components/production_goods_supplier/trading_system.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"

namespace abm::production_goods::supplier {
class ProductionGoodsSupplier final {
    using ProducingSystem = base_goods::supplier::ProducingSystem;
    using Market          = ProductionGoodsMarket;
    using Mediator        = base_goods::supplier::Mediator;
    using CentralMemory   = base_goods::supplier::CentralMemory;

  public:
    [[nodiscard]] explicit ProductionGoodsSupplier(RandomGenerator& masterRng) noexcept
        : producingSystem_{masterRng}, tradingSystem_{masterRng} {
        producingSystem_.acceptMediator(mediator_);
        tradingSystem_.acceptMediator(mediator_);
        mediator_.subscribeMarkupPlan(memory_);
        mediator_.subscribeTradePlan(memory_);
    }

    void post(const AgentID id, const Money totalCost, Market& market) noexcept {
        ASSERT(totalCost >= Money{0.0});

        const auto supply = producingSystem_.produce();
        tradingSystem_.post(id, supply, totalCost, market, mediator_);
    }

    [[nodiscard]] auto calcDesiredEmploy(const HeadCount employee) noexcept -> HeadCount {
        ASSERT(employee >= HeadCount{0.0});

        const auto targetSupply = tradingSystem_.requiresSupply();
        return producingSystem_.calcDesiredEmploy(targetSupply, employee);
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return producingSystem_.workspace(); }

    void trade() noexcept { tradingSystem_.trade(); }

    void addProductionEquip(const GoodsQuantity productionGoods) noexcept {
        producingSystem_.addProducingEquip(productionGoods);
    }

    void endStep(CensusDropBox& dropBox) noexcept {
        tradingSystem_.endStep(mediator_);
        reset(dropBox);
    }

  private:
    void reset(CensusDropBox& dropBox) noexcept {
        memory_.logging(dropBox);
        producingSystem_.reset(dropBox);
        tradingSystem_.reset();
    }

    ProducingSystem producingSystem_;
    TradingSystem   tradingSystem_;
    Mediator        mediator_;
    CentralMemory   memory_;
};
}  // namespace abm::production_goods::supplier

namespace abm {
using ProductionGoodsSupplier = production_goods::supplier::ProductionGoodsSupplier;
}