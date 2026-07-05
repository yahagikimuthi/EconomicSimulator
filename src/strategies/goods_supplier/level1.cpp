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
    ctx.setMyEntry(entryBox.emplace_back(price));
    ctx.setPlan(price, supply, markup);
}

void trade(TradeCtx ctx, Component& comp) {
    auto         myEntry{ctx.getMyEntry()};
    auto&        requestBox = myEntry->requestBox_;
    const double totalDemand{calcTotalDemand(requestBox)};
    const bool   isExcessDemand{totalDemand >= ctx.getSupply()};
    const double salesAmount{
        isExcessDemand ? performRationedTrade(ctx.getSupply(), requestBox)
                       : performFullTrade(requestBox)
    };
    ctx.setInventory(ctx.getSupply() - salesAmount);
    updateLog({comp});
}
}  // namespace goods_supplier