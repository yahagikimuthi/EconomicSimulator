#include "strategies/goods_demander/after_trade.hpp"

#include "world/message.hpp"

namespace goods_demander {
void afterTrade(AfterTradeView view) {
    if (not view.isPosting()) return;
    const auto [entry, myRequest] = view.myRequest();
    view.purchasePlus(entry.price * myRequest.tradeAmount);
}
}  // namespace goods_demander