#include "strategies/goods_demander/level1.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>

#include "strategies/goods_demander/level2.hpp"
#include "world/message.hpp"

namespace goods_demander {
void purchase(
    PurchaseView                               view,
    const double                               availableAsset,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                  maxPurchaseFrequency,
    const int&                                 step
) {
    if (entryBox.empty() or (step % maxPurchaseFrequency != view.myPhase())) {
        view.isPosting(false);
        return;
    }
    const double budget{calcBudget(view, availableAsset)};
    if (budget <= 0.0) {
        view.isPosting(false);
        return;
    }
    auto& pickedEntry = pickEntry(entryBox);
    assert(pickedEntry.price_ > 0.0 && "price is required > 0.0");
    view.entry(pickedEntry, pickedEntry.requestBox_.emplace_back(budget / pickedEntry.price_));
}
}  // namespace goods_demander