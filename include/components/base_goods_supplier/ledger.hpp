#pragma once

#include "components/base_goods_supplier/common.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "values/math.hpp"

namespace abm::base_goods::supplier {
struct ATradeResult final {
    const Price         price;
    const GoodsQuantity demand;
    const GoodsQuantity salesAmount;
};

class Ledger final {
  public:
    [[nodiscard]] Ledger() noexcept = default;

    void makeNewPage(const GoodsQuantity supply) noexcept {
        ASSERT(supply >= GoodsQuantity{0.0});
        supply_    = supply;
        inventory_ = supply;
    }

    [[nodiscard]] auto inventory() const noexcept -> GoodsQuantity {
        ASSERT(inventory_ >= GoodsQuantity{0.0});
        return inventory_;
    }

    [[nodiscard]] auto tradableAmount(const GoodsQuantity demand) const noexcept -> GoodsQuantity {
        ASSERT(demand >= GoodsQuantity{0.0});

        const auto out = min(inventory_, demand);
        ASSERT(out >= GoodsQuantity{0.0});
        return out;
    }

    [[nodiscard]] auto isExcessDemand(const GoodsQuantity demand) const noexcept -> bool {
        ASSERT(demand >= GoodsQuantity{0.0});
        return demand > inventory_;
    }

    void readResult(const ATradeResult& result) noexcept {
        ASSERT(result.salesAmount >= GoodsQuantity{0.0});
        ASSERT(result.price >= Price{0.0});
        ASSERT(result.demand >= GoodsQuantity{0.0});

        inventory_ -= result.salesAmount;
        currentSales_ += result.price * result.salesAmount;
        totalDemand_ += result.demand;
    }

    [[nodiscard]] auto publishResult() const noexcept -> TradeResult {
        const auto out = TradeResult{
            .soldAmount   = supply_ - inventory_,
            .unsoldAmount = inventory_,
            .totalDemand  = totalDemand_,
            .sales        = currentSales_
        };

        ASSERT(out.soldAmount >= GoodsQuantity{0.0});
        ASSERT(out.unsoldAmount >= GoodsQuantity{0.0});
        ASSERT(out.totalDemand >= GoodsQuantity{0.0});
        ASSERT(out.sales >= Money{0.0});
        return out;
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