#include "strategies/goods_demander.hpp"

namespace goods_demander{
    void afterTrade(AfterTradeView view) {
    if (not view.isPosting()) return;
    const auto [entry, myRequest] = view.getMyRequest();
    view.recordPurchase(entry.price_ * myRequest.tradeAmount_);
}
}