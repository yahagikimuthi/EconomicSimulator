#pragma once

#include <tbb/concurrent_vector.h>

#include "components/base_goods_supplier/mediator.hpp"
#include "components/base_goods_supplier/produsing.hpp"
#include "components/consumer_goods_supplier/trading_system.hpp"
#include "util.hpp"
#include "world/goods.hpp"

namespace abm::consumer_goods::supplier {
class ConsumerGoodsSupplier {
    using ProducingSystem = base_goods::supplier::ProducingSystem;
    using Market          = ConsumerGoodsMarket;
    using Mediator        = base_goods::supplier::Mediator;

  public:
    [[nodiscard]] explicit ConsumerGoodsSupplier(
        RandomGenerator& masterRng, const Workspace& workspace
    ) noexcept
        : producingSystem_{masterRng, workspace}, tradingSystem_{masterRng} {}

    void post(const Money totalCost, Market& market) {
        const auto supply = producingSystem_.produce();
        tradingSystem_.post(supply, totalCost, market);
    }

    void trade() { tradingSystem_.trade(); }

    void endStep() { tradingSystem_.endStep(mediator_); }

  private:
    ProducingSystem producingSystem_;
    TradingSystem   tradingSystem_;
    Mediator        mediator_;
};
}  // namespace abm::consumer_goods::supplier

namespace abm {
using ConsumerGoodsSupplier = consumer_goods::supplier::ConsumerGoodsSupplier;
}