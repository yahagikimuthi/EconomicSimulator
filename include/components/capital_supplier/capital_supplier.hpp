#pragma once

#include <utility>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/mediator.hpp"
#include "components/base_goods_supplier/produsing.hpp"
#include "components/capital_supplier/trading_system.hpp"
#include "components/common.hpp"
#include "others/util.hpp"
#include "values/goods.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"
#include "world/common.hpp"

namespace abm::capital::supplier {
class CapitalSupplier final {
    using ProducingSystem = base_goods::supplier::ProducingSystem;
    using Mediator        = base_goods::supplier::Mediator;
    using CentralMemory   = base_goods::supplier::CentralMemory;
    using Workspace       = base_goods::Workspace;

  public:
    [[nodiscard]] explicit CapitalSupplier(RandomGenerator& masterRng) noexcept
        : producingSystem_{masterRng}, tradingSystem_{masterRng} {}

    void setMediator() noexcept {
        producingSystem_.acceptMediator(mediator_);
        tradingSystem_.acceptMediator(mediator_);
        mediator_.subscribeMarkupPlan(memory_);
        mediator_.subscribeTradePlan(memory_);
        mediator_.subscribeTradeResult(memory_);
        mediator_.subscribeTradeResult(producingSystem_);
    }

    [[nodiscard]] auto planAndExpectSales(const Money totalCost) noexcept -> Money {
        ASSERT(totalCost.isZeroOrMore());
        const auto supply = producingSystem_.produce();
        tradingSystem_.plan(supply, totalCost, mediator_);
        return salesForecast();
    }

    void post(const AgentID id, Market& market) noexcept { tradingSystem_.post(id, market); }

    [[nodiscard]] auto calcDesiredEmploy(const HeadCount employee) noexcept -> HeadCount {
        ASSERT(employee >= HeadCount{0.0});

        const auto targetSupply = tradingSystem_.requiresSupply();
        return producingSystem_.calcDesiredEmploy(targetSupply, employee);
    }

    [[nodiscard]] auto requiresCapital() noexcept -> GoodsQuantity {
        const auto requiresSupply = tradingSystem_.requiresSupply();
        return producingSystem_.calcDesiredCapital(requiresSupply);
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return producingSystem_.workspace(); }

    void trade() noexcept { tradingSystem_.trade(); }

    void addCapitalEquip(const GoodsQuantity capital) noexcept {
        producingSystem_.addProducingEquip(capital);
    }

    template <AssetPlusFn F>
    void endStep(F&& assetPlus, CensusDropBox& dropBox) noexcept {
        tradingSystem_.endStep(std::forward<F>(assetPlus), mediator_);
        reset(dropBox);
    }

    [[nodiscard]] auto salesForecast() const noexcept -> Money { return memory_.lastSales(); }

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
}  // namespace abm::capital::supplier

namespace abm {
using CapitalSupplier = capital::supplier::CapitalSupplier;
}