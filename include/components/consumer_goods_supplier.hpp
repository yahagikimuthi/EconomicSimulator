#pragma once

#include <utility>

#include "components/base_supplier/common.hpp"
#include "components/base_supplier/mediator.hpp"
#include "components/base_supplier/produsing.hpp"
#include "components/base_supplier/trading_system.hpp"
#include "components/common.hpp"
#include "components/finance/finance.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"
#include "world/common.hpp"

namespace abm::consumer_goods::supplier {
class ConsumerGoodsSupplier final {
    using TradingSystem   = base_goods::supplier::TradingSystem;
    using ProducingSystem = base_goods::supplier::ProducingSystem;
    using Mediator        = base_goods::supplier::Mediator;
    using CentralMemory   = base_goods::supplier::CentralMemory;
    using Workspace       = base_goods::Workspace;
    using Market          = base_goods::Market;

  public:
    explicit ConsumerGoodsSupplier(RandomGenerator& masterRng) noexcept
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

    [[nodiscard]] auto planAndExpectSales(const Money totalCost) noexcept -> Money {
        const auto supply = producingSystem_.produce();
        tradingSystem_.plan(supply, totalCost, mediator_);
        return -salesForecast();
    }

    void beginingMonth(const Money totalCost) noexcept {
        const auto supply = producingSystem_.produce();
        tradingSystem_.plan(supply, totalCost, mediator_);
    }

    void post(const AgentID id, Market& market) noexcept { tradingSystem_.post(id, market); }
    void trade(FirmFinance& finance) noexcept { tradingSystem_.trade(finance); }

    template <AssetPlusFn F>
    void endStep(F&& assetPlus, CensusDropBox& dropBox) noexcept {
        tradingSystem_.endStep(std::forward<F>(assetPlus), mediator_);
        reset(dropBox);
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return producingSystem_.workspace(); }

    void addCapitalEquip(const GoodsQuantity capital) noexcept {
        ASSERT(capital.isZeroOrMore());
        producingSystem_.addProducingEquip(capital);
    }

    [[nodiscard]] auto salesForecast() const noexcept -> Money { return memory_.lastSales(); }

    [[nodiscard]] auto requiresCapital() noexcept -> GoodsQuantity {
        const auto requiresSupply = tradingSystem_.requiresSupply();
        return producingSystem_.calcDesiredCapital(requiresSupply);
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