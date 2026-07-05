#pragma once

#include "components/goods_supplier.hpp"

#include "config/init_setup.hpp"

namespace goods_supplier {
struct CalcSupplyCtx {
    CalcSupplyCtx(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto firmProductPower() const -> double {
        return comp_.production_.firmProductPower_;
    }
    [[nodiscard]] auto sumEmployeeProductPower() const -> double {
        return comp_.production_.sumEmployeeProductPower_;
    }
    [[nodiscard]] auto inventory() const -> double { return comp_.production_.inventory_; }

  private:
    Component& comp_;
};
[[nodiscard]] auto calcSupply(const CalcSupplyCtx ctx) -> double;

struct CalcMarkupCtx {
    CalcMarkupCtx(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto markupAdjustVol() const -> double {
        return comp_.parameter_.markupAdjustmentVolatility_;
    }
    [[nodiscard]] auto lastMarkup() const -> double { return comp_.log_.markup_; }
    [[nodiscard]] auto isSold() const -> bool { return comp_.log_.isSold_; }

  private:
    Component& comp_;
};
[[nodiscard]] auto calcMarkup(
    const CalcMarkupCtx ctx, const double epsilonMarkup = config::goods_supplier::epsilonMarkup
) -> double;

[[nodiscard]] auto judgePrice(
    const double markup,
    const double totalCost,
    const double epsilonPrice = config::goods_supplier::epsilonPrice
) -> double;
}  // namespace goods_supplier