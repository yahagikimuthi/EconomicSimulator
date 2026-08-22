#pragma once

#include <tbb/concurrent_vector.h>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/mediator.hpp"
#include "components/base_goods_supplier/produsing.hpp"
#include "components/consumer_goods_supplier/trading_system.hpp"
#include "components/others.hpp"
#include "core/util.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/common.hpp"
#include "world/goods.hpp"

namespace abm::consumer_goods::supplier {
class ConsumerGoodsSupplier final {
    using ProducingSystem = base_goods::supplier::ProducingSystem;
    using Market          = ConsumerGoodsMarket;
    using Mediator        = base_goods::supplier::Mediator;
    using CentralMemory   = base_goods::supplier::CentralMemory;

  public:
    [[nodiscard]] explicit ConsumerGoodsSupplier(RandomGenerator& masterRng) noexcept
        : producingSystem_{masterRng}, tradingSystem_{masterRng} {}

    void setMediator() noexcept {
        producingSystem_.acceptMediator(mediator_);
        tradingSystem_.acceptMediator(mediator_);
        mediator_.subscribeMarkupPlan(memory_);
        mediator_.subscribeTradePlan(memory_);
    }

    [[nodiscard]] auto calcDesiredEmploy(const HeadCount employee) noexcept -> HeadCount {
        const auto requiresSupply = tradingSystem_.requiresSupply();
        return producingSystem_.calcDesiredEmploy(requiresSupply, employee);
    }

    void post(const Money totalCost, Market& market) noexcept {
        const auto supply = producingSystem_.produce();
        tradingSystem_.post(supply, totalCost, market, mediator_);
    }
    void trade() noexcept { tradingSystem_.trade(); }

    template <AssetPlusFn F>
    void endStep(F&& assetPlus, CensusDropBox& dropBox) noexcept {
        tradingSystem_.endStep(std::forward<F>(assetPlus), mediator_);
        reset(dropBox);
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return producingSystem_.workspace(); }

  private:
    void reset(CensusDropBox& dropBox) noexcept {
        tradingSystem_.reset();
        producingSystem_.reset(dropBox);
        memory_.logging(dropBox);
    }

    ProducingSystem producingSystem_;
    TradingSystem   tradingSystem_;
    Mediator        mediator_;
    CentralMemory   memory_;
};
}  // namespace abm::consumer_goods::supplier

namespace abm {
using ConsumerGoodsSupplier = consumer_goods::supplier::ConsumerGoodsSupplier;
}