#pragma once

#include <algorithm>

#include "components/base_goods_supplier/common.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"

namespace abm::base_goods::supplier {
struct ATradeResult final {
    const Price         price;
    const GoodsQuantity demand;
    const GoodsQuantity salesAmount;
};

class Ledger final {
  public:
    [[nodiscard]] Ledger() noexcept = default;

    [[nodiscard]] auto inventory() const noexcept -> GoodsQuantity { return inventory_; }
    [[nodiscard]] auto canTradeAmount(const GoodsQuantity demand) const noexcept -> GoodsQuantity {
        const auto out = std::max(inventory_.value(), demand.value());
        return GoodsQuantity{out};
    }
    [[nodiscard]] auto isExcessDemand(const GoodsQuantity demand) const noexcept -> bool {
        return demand > inventory_;
    }
    void readResult(const ATradeResult& result) noexcept {
        inventory_ -= result.salesAmount;
        currentSales_ += result.price * result.salesAmount;
        totalDemand_ += result.demand;
    }

    [[nodiscard]] auto publishResult() const noexcept -> TradeResult {
        return {
            .soldAmount   = supply_ - inventory_,
            .unsoldAmount = inventory_,
            .totalDemand  = totalDemand_,
            .sales        = currentSales_
        };
    }

    void reset() noexcept {
        supply_       = GoodsQuantity{0.0};
        inventory_    = GoodsQuantity{0.0};
        currentSales_ = Money{0.0};
        totalDemand_  = GoodsQuantity{0.0};
    }

  private:
    GoodsQuantity supply_{0.0};
    GoodsQuantity inventory_{0.0};
    Money         currentSales_{0.0};
    GoodsQuantity totalDemand_{0.0};
};
}  // namespace abm::base_goods::supplier