#pragma once

#include <tbb/concurrent_vector.h>

#include "components/base_goods_supplier/mediator.hpp"
#include "components/base_goods_supplier/produsing.hpp"
#include "components/production_goods_supplier/trading_system.hpp"
#include "util.hpp"
#include "world/goods.hpp"

namespace abm::production_goods::supplier {
class ProductionGoodsSupplier final {
    using ProducingSystem = base_goods::supplier::ProducingSystem;
    using Market          = ProductionGoodsMarket;
    using Mediator        = base_goods::supplier::Mediator;

  public:
    [[nodiscard]] explicit ProductionGoodsSupplier(
        RandomGenerator& masterRng, const Workspace& workspace
    ) noexcept
        : producingSystem_{masterRng, workspace}, tradingSystem_{masterRng} {}

    void post(const AgentID id, const Money totalCost, Market& market) noexcept {
        const auto supply = producingSystem_.produce();
        tradingSystem_.post(id, supply, totalCost, market);
    }

    void trade() noexcept { tradingSystem_.trade(); }

    void endStep() noexcept { tradingSystem_.endStep(mediator_); }

  private:
    ProducingSystem producingSystem_;
    TradingSystem   tradingSystem_;
    Mediator        mediator_;
};
}  // namespace abm::production_goods::supplier

namespace abm {
using ProductionGoodsSupplier = production_goods::supplier::ProductionGoodsSupplier;
}