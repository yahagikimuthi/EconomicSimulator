#include "strategies/goods_supplier.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <cmath>

#include "config.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace goods_supplier {
struct CalcSupplyView {
    explicit CalcSupplyView(const PostGoodsView& parentView) : comp_{parentView.comp_} {}

    [[nodiscard]] auto firmProductPower() const -> double {
        return comp_.production_.firmProductPower_;
    }
    [[nodiscard]] auto sumEmployeeProductPower() const -> double {
        return comp_.production_.sumEmployeeProductPower_;
    }
    [[nodiscard]] auto inventory() const -> double { return comp_.production_.inventory_; }

  private:
    const Component& comp_;
};

struct CalcMarkupView {
    explicit CalcMarkupView(const PostGoodsView& parentView) : comp_{parentView.comp_} {}

    [[nodiscard]] auto markupAdjustVol() const -> double {
        return comp_.parameter_.markupAdjustmentVolatility_;
    }
    [[nodiscard]] auto lastMarkup() const -> double { return comp_.log_.markup_; }
    [[nodiscard]] auto isSold() const -> bool { return comp_.log_.isSold_; }

  private:
    Component& comp_;
};

namespace {
[[nodiscard]] auto calcSupply(const CalcSupplyView view) -> double {
    const double out{(view.firmProductPower() * view.sumEmployeeProductPower()) + view.inventory()};
    assert(out >= 0.0 && "supply amount is required >= 0");
    return out;
}

[[nodiscard]] auto calcMarkup(
    const CalcMarkupView view, const double epsilonMarkup = config::goods_supplier::epsilonMarkup
) -> double {
    const double alpha{std::abs(helper::randNormal(0.0, view.markupAdjustVol()))};
    const double nextMarkup{view.lastMarkup() + (view.isSold() ? alpha : -alpha)};
    return std::max(epsilonMarkup, nextMarkup);
}

[[nodiscard]] auto judgePrice(
    const double markup,
    const double totalCost,
    const double epsilonPrice = config::goods_supplier::epsilonPrice
) -> double {
    assert(markup > 0.0 && "markup is required > 0");
    assert(totalCost >= 0.0 && "total cost is required > 0");
    const double price{totalCost * markup};
    return std::max(epsilonPrice, price);
}
}  // namespace

void postGoods(
    PostGoodsView view, const double totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const double supply{calcSupply(CalcSupplyView{view})};
    if (supply == 0.0) {
        view.isPosting(false);
        return;
    }
    view.isPosting(true);
    const double markup{calcMarkup(CalcMarkupView{view})};
    const double price{judgePrice(markup, totalCost)};
    view.myEntry(entryBox.emplace_back(price, supply));
    view.plan(price, supply, markup);
}
}  // namespace goods_supplier