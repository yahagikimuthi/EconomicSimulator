#include "strategies/goods_demander.hpp"

namespace goods_demander {
void afterTrade(AfterTradeView view) {
    if (not view.isPosting()) return;
    const auto [entry, myRequest] = view.myRequest();
    view.recordPurchase(entry.price_ * myRequest.tradeAmount_);
}
}  // namespace goods_demander