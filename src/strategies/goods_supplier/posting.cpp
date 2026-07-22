#include "strategies/goods_supplier.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <cmath>
#include <pcg_random.hpp>

#include "config.hpp"
#include "core/base.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace goods_supplier {
namespace {
[[nodiscard]] auto markupGuard(
    const double markup, const double epsilon = config::goods_supplier::epsilonMarkup
) -> double {
    return std::max(markup, epsilon);
}

[[nodiscard]] auto priceGuard(
    const double price, const double epsilon = config::goods_supplier::epsilonPrice
) -> double {
    return std::max(price, epsilon);
}

struct [[nodiscard]] CalcMarkupView final : BaseView<Component> {
    using BaseView<Component>::BaseView;

    auto markupAdjustVol() const -> double { return comp_.parameter_.markupAdjustmentVolatility_; }
    auto lastMarkup() const -> double { return comp_.log_.markup_; }
    auto isSold() const -> bool { return comp_.log_.isSold_; }
    auto rng() -> pcg32& { return comp_.rng_; }
};

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
}  // namespace

void postGoods(
    PostGoodsView view, const double totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const double supply{view.laborInput()};
    const double markup{calcMarkup(CalcMarkupView{view})};
    const double price{judgePrice(supply, markup, totalCost)};
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