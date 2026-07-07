#include "strategies/goods_supplier/level1.hpp"

#include <tbb/concurrent_vector.h>

#include "strategies/goods_supplier/level2.hpp"
#include "world/message.hpp"

namespace goods_supplier {
[[nodiscard]] auto isSold(const IsSoldView& view) -> bool {
    if (view.lastSupply() == 0.0) return false;
    return view.inventory() / view.lastSupply() < view.targetInvRatio();
}

void postGoods(
    PostGoodsView view, const double totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const double supply{calcSupply(view)};
    const double markup{calcMarkup(view)};
    const double price{judgePrice(markup, totalCost)};
    view.setMyEntry(entryBox.emplace_back(price, supply));
    view.setPlan(price, supply, markup);
}

void trade(TradeView view) {
    auto         myEntry{view.getMyEntry()};
    auto&        requestBox = myEntry->requestBox_;
    const double totalDemand{calcTotalDemand(requestBox)};
    const bool   isExcessDemand{totalDemand >= view.supply()};
    const double salesAmount{
        isExcessDemand ? performRationedTrade(view.supply(), requestBox)
                       : performFullTrade(requestBox)
    };
    view.setInventory(view.supply() - salesAmount);
    updateLog(view, salesAmount);
}
}  // namespace goods_supplier