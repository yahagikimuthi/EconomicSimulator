#pragma once

#include <cassert>
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
#include "world/drop_box.hpp"

namespace abm::base_goods::supplier {

class BaseGoodsSupplier {
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
    BaseGoodsSupplier(BaseGoodsSupplier&& other) noexcept                   = delete;
    auto operator=(const BaseGoodsSupplier&) noexcept -> BaseGoodsSupplier& = delete;
    auto operator=(BaseGoodsSupplier&&) noexcept -> BaseGoodsSupplier&      = delete;
    ~BaseGoodsSupplier() noexcept                                           = default;

    [[nodiscard]] auto planAndExpectSales(const Budget totalCost) noexcept -> Budget {
        assert(totalCost.isZeroOrMore());
        const auto supply = producingSystem_.produce();
        tradingSystem_.plan(supply, totalCost, mediator_);
        return salesForecast();
    }

    void post(const AgentID id, Market& market) noexcept { tradingSystem_.post(id, market); }

    template <DepositFn F>
    void trade(F&& depositFn) noexcept {
        tradingSystem_.trade(std::forward<F>(depositFn), mediator_);
    }

    [[nodiscard]] auto calcDesiredEmploy(const HeadCount employee) noexcept -> HeadCount {
        assert(employee.isZeroOrMore());

        const auto targetSupply = tradingSystem_.requiresSupply();
        return producingSystem_.calcDesiredEmploy(targetSupply, employee);
    }

    [[nodiscard]] auto requiresCapital() noexcept -> GoodsQuantity {
        const auto requiresSupply = tradingSystem_.requiresSupply();
        return producingSystem_.calcDesiredCapital(requiresSupply);
    }

    [[nodiscard]] auto workspace() noexcept -> Workspace& { return producingSystem_.workspace(); }

    [[nodiscard]] auto makeAddCapitalFn() noexcept -> AddGoodsFn auto {
        return [&](const GoodsQuantity capital) noexcept -> void {
            producingSystem_.addProducingEquip(capital);
        };
    }

    [[nodiscard]] auto salesForecast() const noexcept -> Budget {
        return static_cast<Budget>(memory_.lastSales());
    }

    void logging(BaseGoodsDropBox auto& dropBox) noexcept {
        producingSystem_.logging(dropBox);
        memory_.logging(dropBox);
    }

  private:
    void setMediator() noexcept {
        tradingSystem_.acceptMediator(mediator_);
        mediator_.subscribeMarkupPlan(memory_);
        mediator_.subscribeTradePlan(memory_);
        mediator_.subscribeTradeResult(memory_);
        mediator_.subscribeTradeResult(producingSystem_);
    }

    ProducingSystem producingSystem_;
    TradingSystem   tradingSystem_;
    Mediator        mediator_;
    CentralMemory   memory_;
};
}  // namespace abm::base_goods::supplier

namespace abm {
class CapitalSupplier : public base_goods::supplier::BaseGoodsSupplier {
    using BaseGoodsSupplier::BaseGoodsSupplier;
};
class GoodsSupplier : public base_goods::supplier::BaseGoodsSupplier {
    using BaseGoodsSupplier::BaseGoodsSupplier;
};
}  // namespace abm