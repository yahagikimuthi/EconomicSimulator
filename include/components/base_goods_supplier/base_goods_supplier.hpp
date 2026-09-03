#pragma once

#include <utility>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/mediator.hpp"
#include "components/base_goods_supplier/produsing.hpp"
#include "components/base_goods_supplier/trading_system.hpp"
#include "components/common.hpp"
#include "others/util.hpp"
#include "values/goods.hpp"
#include "values/labor.hpp"
#include "world/base_goods.hpp"
#include "world/common.hpp"
#include "world/drop_box.hpp"

namespace abm::base_goods::supplier {

template <EMarket SupplyGoodsT>
class BaseGoodsSupplier final {
    using MarketT = Market<SupplyGoodsT>;

  public:
    explicit BaseGoodsSupplier(RandomGenerator& masterRng) noexcept
        : producingSystem_{masterRng}, tradingSystem_{masterRng} {
        setMediator();
    }
    BaseGoodsSupplier(const BaseGoodsSupplier& other) noexcept
        : producingSystem_{other.producingSystem_},
          tradingSystem_{other.tradingSystem_},
          memory_{other.memory_} {
        setMediator();
    }
    BaseGoodsSupplier(BaseGoodsSupplier&& other) noexcept
        : producingSystem_{std::move(other.producingSystem_)},
          tradingSystem_{std::move(other.tradingSystem_)},
          memory_{other.memory_} {
        setMediator();
    }
    auto operator=(const BaseGoodsSupplier&) -> BaseGoodsSupplier& = delete;
    auto operator=(BaseGoodsSupplier&&) -> BaseGoodsSupplier&      = delete;
    ~BaseGoodsSupplier() noexcept                                  = default;

    [[nodiscard]] auto planAndExpectSales(const Money totalCost) noexcept -> Budget {
        ASSERT(totalCost.isZeroOrMore());
        const auto supply = producingSystem_.produce();
        tradingSystem_.plan(supply, totalCost, mediator_);
        return static_cast<Budget>(salesForecast());
    }

    void post(const AgentID id, MarketT& market) noexcept { tradingSystem_.post(id, market); }

    [[nodiscard]] auto calcDesiredEmploy(const HeadCount employee) noexcept -> HeadCount {
        ASSERT(employee.isZeroOrMore());

        const auto targetSupply = tradingSystem_.requiresSupply();
        return producingSystem_.calcDesiredEmploy(targetSupply, employee);
    }

    [[nodiscard]] auto requiresCapital() noexcept -> GoodsQuantity {
        const auto requiresSupply = tradingSystem_.requiresSupply();
        return producingSystem_.calcDesiredCapital(requiresSupply);
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return producingSystem_.workspace(); }

    template <DepositFn F>
    void trade(F&& depositFn) noexcept {
        tradingSystem_.trade(std::forward<F>(depositFn));
    }

    void addCapitalEquip(const GoodsQuantity capital) noexcept {
        producingSystem_.addProducingEquip(capital);
    }

    void endTrading() noexcept { tradingSystem_.endTrading(mediator_); }

    [[nodiscard]] auto salesForecast() const noexcept -> Money { return memory_.lastSales(); }

  private:
    void reset(CensusDropBox& dropBox) noexcept {
        memory_.logging(dropBox);
        producingSystem_.reset(dropBox);
    }

    void setMediator() noexcept {
        producingSystem_.acceptMediator(mediator_);
        tradingSystem_.acceptMediator(mediator_);
        mediator_.subscribeMarkupPlan(memory_);
        mediator_.subscribeTradePlan(memory_);
        mediator_.subscribeTradeResult(memory_);
        mediator_.subscribeTradeResult(producingSystem_);
    }

    ProducingSystem             producingSystem_;
    TradingSystem<SupplyGoodsT> tradingSystem_;
    Mediator                    mediator_;
    CentralMemory               memory_;
};
}  // namespace abm::base_goods::supplier

namespace abm {
using CapitalSupplier = base_goods::supplier::BaseGoodsSupplier<EMarket::Capital>;
using GoodsSupplier   = base_goods::supplier::BaseGoodsSupplier<EMarket::Goods>;
}  // namespace abm