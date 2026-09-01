#pragma once

#include <algorithm>

#include "components/base_goods_supplier/common.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"

namespace abm::capital::supplier {
struct ATradeResult final {
    const Price         price;
    const GoodsQuantity demand;
    const GoodsQuantity salesAmount;
};

class Ledger final {
    using TradeResult = base_goods::supplier::TradeResult;

  public:
    Ledger() noexcept = default;

    void makeNewPage(const GoodsQuantity supply) noexcept {
        ASSERT(supply.isZeroOrMore());
        inventory_ = supply;
    }

    [[nodiscard]] auto inventory() const noexcept -> GoodsQuantity {
        ASSERT(inventory_.isZeroOrMore());
        return inventory_;
    }

    [[nodiscard]] auto tradableAmount(const GoodsQuantity demand) const noexcept -> GoodsQuantity {
        ASSERT(demand.isZeroOrMore());

        const auto out = std::min(inventory_, demand);
        ASSERT(out.isZeroOrMore());
        return out;
    }

    [[nodiscard]] auto isExcessDemand(const GoodsQuantity demand) const noexcept -> bool {
        ASSERT(demand.isZeroOrMore());
        return demand > inventory_;
    }

    [[nodiscard]] auto publishResult(const ATradeResult& result) noexcept -> TradeResult {
        ASSERT(result.demand.isZeroOrMore());
        ASSERT(result.price.isPositive());
        ASSERT(result.salesAmount.isZeroOrMore());

        const auto out = TradeResult{
            .soldAmount   = result.salesAmount,
            .unsoldAmount = inventory_ - result.salesAmount,
            .totalDemand  = result.demand,
            .sales        = result.salesAmount * result.price
        };

        ASSERT(out.soldAmount.isZeroOrMore());
        ASSERT(out.unsoldAmount.isZeroOrMore());
        ASSERT(out.totalDemand.isZeroOrMore());
        ASSERT(out.sales.isZeroOrMore());
        return out;
    }

  private:
    GoodsQuantity inventory_{0.0};
};
}  // namespace abm::capital::supplier