#include "strategies/goods_supplier/level2.hpp"

#include <cmath>

#include "helper.hpp"

namespace goods_supplier {
[[nodiscard]] auto calcSupply(const CalcSupplyCtx ctx) -> double {
    return (ctx.firmProductPower() * ctx.sumEmployeeProductPower()) + ctx.inventory();
}

[[nodiscard]] auto calcMarkup(const CalcMarkupCtx ctx, const double epsilonMarkup) -> double {
    const double alpha{std::abs(helper::randNormal(0.0, ctx.markupAdjustVol(), -1.0, 1.0))};
    const double nextMarkup{ctx.lastMarkup() * (ctx.isSold() ? 1.0 + alpha : 1.0 - alpha)};
    return std::max(epsilonMarkup, nextMarkup);
}

[[nodiscard]] auto judgePrice(
    const double markup, const double totalCost, const double epsilonPrice
) -> double {
    const double price{totalCost * markup};
    return std::max(epsilonPrice, price);
}
}  // namespace goods_supplier