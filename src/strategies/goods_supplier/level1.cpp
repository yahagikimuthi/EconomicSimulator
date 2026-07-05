#include "strategies/goods_supplier/level1.hpp"

#include <tbb/concurrent_vector.h>

#include "strategies/goods_supplier/level2.hpp"
#include "world/message.hpp"

namespace goods_supplier {
[[nodiscard]] auto isSold(const IsSoldCtx& ctx) -> bool {
    if (ctx.lastSupply() == 0.0) return false;
    return ctx.inventory() / ctx.lastSupply() < ctx.targetInvRatio();
}

void postGoods(
    PostGoodsCtx                               ctx,
    const double                               totalCost,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    Component&                                 comp
) {
    const double supply{calcSupply({comp})};
    const double markup{calcMarkup({comp})};
    const double price{judgePrice(markup, totalCost)};
    ctx.setMyEntry(entryBox.emplace_back(price, supply));
    ctx.setPlan(price, supply, markup);
}
}  // namespace goods_supplier