#include "strategies/goods_supplier.hpp"

#include <tbb/concurrent_vector.h>
#include <cassert>
#include <cmath>
#include <config/init_setup.hpp>

#include "helper.hpp"
#include "world/message.hpp"

namespace goods_supplier {
struct CalcSupplyView {
    CalcSupplyView(const PostGoodsView& parentView) : comp_{parentView.comp_} {}

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
    CalcMarkupView(const PostGoodsView& parentView) : comp_{parentView.comp_} {}

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
    return (view.firmProductPower() * view.sumEmployeeProductPower()) + view.inventory();
}

[[nodiscard]] auto calcMarkup(
    const CalcMarkupView view, const double epsilonMarkup = config::goods_supplier::epsilonMarkup
) -> double {
    const double alpha{std::abs(helper::randNormal(0.0, view.markupAdjustVol(), -1.0, 1.0))};
    const double nextMarkup{view.lastMarkup() * (view.isSold() ? 1.0 + alpha : 1.0 - alpha)};
    return std::max(epsilonMarkup, nextMarkup);
}

[[nodiscard]] auto judgePrice(
    const double markup,
    const double totalCost,
    const double epsilonPrice = config::goods_supplier::epsilonPrice
) -> double {
    assert(markup > 0.0 && "markup is required > 0");
    assert(totalCost > 0.0 && "total cost is required > 0");
    const double price{totalCost * markup};
    return std::max(epsilonPrice, price);
}
}  // namespace

void postGoods(
    PostGoodsView view, const double totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const double supply{calcSupply(view)};
    const double markup{calcMarkup(view)};
    const double price{judgePrice(markup, totalCost)};
    view.setMyEntry(entryBox.emplace_back(price, supply));
    view.setPlan(price, supply, markup);
}
}  // namespace goods_supplier