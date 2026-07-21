#include "strategies/goods_demander.hpp"

#include "world/message.hpp"

namespace goods_demander {
void afterTrade(AfterTradeView view) {
    if (not view.isPosting()) return;
    const auto [entry, myRequest] = view.myRequest();
    view.purchasePlus(entry.price_ * myRequest.tradeAmount_);
}
}  // namespace goods_demander