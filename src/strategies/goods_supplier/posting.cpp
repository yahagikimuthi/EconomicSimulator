#include "strategies/goods_supplier/posting.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <cmath>
#include <pcg_random.hpp>

#include "helper.hpp"
#include "world/message.hpp"

namespace goods_supplier::internal {
[[nodiscard]] auto markupGuard(const double markup, const double epsilon) -> double {
    return std::max(markup, epsilon);
}

[[nodiscard]] auto priceGuard(const double price, const double epsilon) -> double {
    return std::max(price, epsilon);
}

[[nodiscard]] auto calcMarkup(CalcMarkupView view) -> double {
    const double alpha{std::abs(helper::randNormal(view.rng(), 0.0, view.markupAdjustVol()))};
    const double nextMarkup{view.lastMarkup() + (view.isSold() ? alpha : -alpha)};
    return markupGuard(nextMarkup);
}

[[nodiscard]] auto judgePrice(const double supply, const double markup, const double totalCost)
    -> double {
    assert(markup > 0.0 && "markup is required > 0");
    const double avgCost{(supply != 0.0) ? totalCost / supply : 0.0};
    const double price{avgCost * (1.0 + markup)};
    return priceGuard(price);
}
}  // namespace goods_supplier::internal

namespace goods_supplier {
void postGoods(
    PostGoodsView view, const double totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const double supply{view.laborInput()};
    const double markup{internal::calcMarkup(internal::CalcMarkupView{view})};
    const double price{internal::judgePrice(supply, markup, totalCost)};
    view.plan(price, supply, markup);

    // markupやprice自体は供給量0でも計算対象
    if (supply == 0.0) {
        view.isPosting(false);
        return;
    }
    view.isPosting(true);
    view.myEntry(entryBox.emplace_back(price, supply));
}
}  // namespace goods_supplier