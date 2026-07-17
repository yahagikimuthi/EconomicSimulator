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

struct CalcSupplyView final : BaseView<Component> {
    using BaseView<Component>::BaseView;

    auto firmProductPower() const -> double { return comp_.production_.firmProductPower_; }
    auto sumEmployeeProductPower() const -> double {
        return comp_.production_.sumEmployeeProductPower_;
    }
    auto inventory() const -> double { return comp_.production_.inventory_; }
};

[[nodiscard]] auto calcSupply(const CalcSupplyView& view) -> double {
    const double out{(view.firmProductPower() * view.sumEmployeeProductPower()) + view.inventory()};
    assert(out >= 0.0 && "supply amount is required >= 0");
    return out;
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

struct [[nodiscard]] CalcAvgCostView final : BaseView<Component> {
    using BaseView<Component>::BaseView;

    auto firmProductPower() const -> double { return comp_.production_.firmProductPower_; }
    auto sumEmployeeProductPower() const -> double {
        return comp_.production_.sumEmployeeProductPower_;
    }
};

[[nodiscard]] auto calcAvgCost(const CalcAvgCostView& view, const double totalCost) -> double {
    assert(totalCost >= 0.0 && "total cost is required > 0");

    const double sumProductPower{view.sumEmployeeProductPower() * view.firmProductPower()};
    const double avgCost{(sumProductPower != 0.0) ? totalCost / sumProductPower : 0.0};
    assert(avgCost >= 0.0 && "average cost is required >= 0");
    return avgCost;
}

struct [[nodiscard]] JudgePriceView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
};

[[nodiscard]] auto judgePrice(JudgePriceView view, const double markup, const double totalCost)
    -> double {
    assert(markup > 0.0 && "markup is required > 0");
    const double avgCost{calcAvgCost(CalcAvgCostView{view}, totalCost)};
    const double price{avgCost * (1.0 + markup)};
    return priceGuard(price);
}
}  // namespace

void postGoods(
    PostGoodsView view, const double totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const double supply{calcSupply(CalcSupplyView{view})};
    const double markup{calcMarkup(CalcMarkupView{view})};
    const double price{judgePrice(JudgePriceView{view}, markup, totalCost)};
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