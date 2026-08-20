#pragma once

#include <tbb/concurrent_vector.h>

#include "components/base_goods_supplier/mediator.hpp"
#include "components/base_goods_supplier/produsing.hpp"
#include "components/consumer_goods_supplier/trading_system.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "util.hpp"
#include "world/goods.hpp"

namespace abm::consumer_goods::supplier {
class ConsumerGoodsSupplier final {
    using ProducingSystem = base_goods::supplier::ProducingSystem;
    using Market          = ConsumerGoodsMarket;
    using Mediator        = base_goods::supplier::Mediator;

  public:
    [[nodiscard]] explicit ConsumerGoodsSupplier(RandomGenerator& masterRng) noexcept
        : producingSystem_{masterRng}, tradingSystem_{masterRng} {
        producingSystem_.acceptMediator(mediator_);
        tradingSystem_.acceptMediator(mediator_);
    }

    [[nodiscard]] auto calcDesiredEmploy(const HeadCount employee) noexcept -> HeadCount {
        const auto requiresSupply = tradingSystem_.requiresSupply();
        return producingSystem_.calcDesiredEmploy(requiresSupply, employee);
    }

    void post(const Money totalCost, Market& market) noexcept {
        const auto supply = producingSystem_.produce();
        tradingSystem_.post(supply, totalCost, market);
    }
    void trade() noexcept { tradingSystem_.trade(); }
    void endStep() noexcept { tradingSystem_.endStep(mediator_); }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return producingSystem_.workspace(); }

  private:
    ProducingSystem producingSystem_;
    TradingSystem   tradingSystem_;
    Mediator        mediator_;
};
}  // namespace abm::consumer_goods::supplier

namespace abm {
using ConsumerGoodsSupplier = consumer_goods::supplier::ConsumerGoodsSupplier;
}