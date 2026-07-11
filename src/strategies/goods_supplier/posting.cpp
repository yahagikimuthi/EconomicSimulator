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

struct JudgePriceView {
    explicit JudgePriceView(const PostGoodsView& parentView) : comp_{parentView.comp_} {}

  private:
    Component& comp_;
    friend struct CalcAvgCostView;
};

struct CalcAvgCostView {
    explicit CalcAvgCostView(const JudgePriceView& parentView) : comp_{parentView.comp_} {}

    [[nodiscard]] auto firmProductPower() const -> double {
        return comp_.production_.firmProductPower_;
    }
    [[nodiscard]] auto sumEmployeeProductPower() const -> double {
        return comp_.production_.sumEmployeeProductPower_;
    }

  private:
    Component& comp_;
};

namespace {
[[nodiscard]] auto calcSupply(const CalcSupplyView view) -> double {
    const double out{(view.firmProductPower() * view.sumEmployeeProductPower()) + view.inventory()};
    assert(out >= 0.0 && "supply amount is required >= 0");
    return out;
}

[[nodiscard]] auto markupGuard(
    const double markup, const double epsilon = config::goods_supplier::epsilonMarkup
) -> double {
    return std::max(markup, epsilon);
}

[[nodiscard]] auto calcMarkup(const CalcMarkupView view) -> double {
    const double alpha{std::abs(helper::randNormal(0.0, view.markupAdjustVol()))};
    const double nextMarkup{view.lastMarkup() + (view.isSold() ? alpha : -alpha)};
    return markupGuard(nextMarkup);
}

[[nodiscard]] auto calcAvgCost(
    const CalcAvgCostView view, const double totalCost, const double employeeCnt
) -> double {
    assert(totalCost >= 0.0 && "total cost is required > 0");
    const double avgWage{(employeeCnt != 0.0) ? totalCost / employeeCnt : 0.0};
    const double avgProductivity{
        (employeeCnt != 0.0)
            ? view.sumEmployeeProductPower() * view.firmProductPower() / employeeCnt
            : view.firmProductPower()
    };
    const double avgCost{(avgProductivity != 0.0) ? avgWage / avgProductivity : 0.0};
    assert(avgCost >= 0.0 && "average cost is required >= 0");
    return avgCost;
}

[[nodiscard]] auto priceGuard(
    const double price, const double epsilon = config::goods_supplier::epsilonPrice
) -> double {
    return std::max(price, epsilon);
}

[[nodiscard]] auto judgePrice(
    const JudgePriceView view, const double markup, const double totalCost, const double employeeCnt
) -> double {
    assert(markup > 0.0 && "markup is required > 0");
    const double avgCost{calcAvgCost(CalcAvgCostView{view}, totalCost, employeeCnt)};
    const double price{avgCost * (1.0 + markup)};
    return priceGuard(price);
}
}  // namespace

void postGoods(
    PostGoodsView                              view,
    const double                               totalCost,
    const double                               employeeCnt,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    const double supply{calcSupply(CalcSupplyView{view})};
    const double markup{calcMarkup(CalcMarkupView{view})};
    const double price{judgePrice(JudgePriceView{view}, markup, totalCost, employeeCnt)};
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