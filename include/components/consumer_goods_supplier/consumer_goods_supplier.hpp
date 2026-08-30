#pragma once

#include <utility>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/mediator.hpp"
#include "components/base_goods_supplier/produsing.hpp"
#include "components/common.hpp"
#include "components/consumer_goods_supplier/trading_system.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"
#include "world/common.hpp"

namespace abm::consumer_goods::supplier {
class ConsumerGoodsSupplier final {
    using ProducingSystem = base_goods::supplier::ProducingSystem;
    using Mediator        = base_goods::supplier::Mediator;
    using CentralMemory   = base_goods::supplier::CentralMemory;
    using Workspace       = base_goods::Workspace;

  public:
    [[nodiscard]] explicit ConsumerGoodsSupplier(RandomGenerator& masterRng) noexcept
        : producingSystem_{masterRng}, tradingSystem_{masterRng} {}

    void setMediator() noexcept {
        producingSystem_.acceptMediator(mediator_);
        tradingSystem_.acceptMediator(mediator_);
        mediator_.subscribeMarkupPlan(memory_);
        mediator_.subscribeTradePlan(memory_);
        mediator_.subscribeTradeResult(memory_);
        mediator_.subscribeTradeResult(producingSystem_);
    }

    [[nodiscard]] auto calcDesiredEmploy(const HeadCount employee) noexcept -> HeadCount {
        const auto requiresSupply = tradingSystem_.requiresSupply();
        return producingSystem_.calcDesiredEmploy(requiresSupply, employee);
    }

    [[nodiscard]] auto planAndRequestBudget(const Money totalCost) noexcept -> Money {
        const auto supply = producingSystem_.produce();
        tradingSystem_.plan(supply, totalCost, mediator_);
        return -salesForecast();
    }
    void revisePlan(const Money _) noexcept {}

    void beginingMonth(const Money totalCost) noexcept {
        const auto supply = producingSystem_.produce();
        tradingSystem_.plan(supply, totalCost, mediator_);
    }

    void post(Market& market) noexcept { tradingSystem_.post(market); }
    void trade() noexcept { tradingSystem_.trade(); }

    template <AssetPlusFn F>
    void endStep(F&& assetPlus, CensusDropBox& dropBox) noexcept {
        tradingSystem_.endStep(std::forward<F>(assetPlus), mediator_);
        reset(dropBox);
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return producingSystem_.workspace(); }

    void addCapitalEquip(const GoodsQuantity capitalGoods) noexcept {
        ASSERT(capitalGoods >= GoodsQuantity{0.0});
        producingSystem_.addProducingEquip(capitalGoods);
    }

    [[nodiscard]] auto salesForecast() const noexcept -> Money { return memory_.lastSales(); }

    [[nodiscard]] auto requiresCapitalGoods() noexcept -> GoodsQuantity {
        const auto requiresSupply = tradingSystem_.requiresSupply();
        return producingSystem_.calcDesiredCapitalGoods(requiresSupply);
    }

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