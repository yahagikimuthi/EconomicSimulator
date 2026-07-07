#pragma once

#include <tbb/concurrent_vector.h>

#include "components/goods_supplier.hpp"
#include "config/init_setup.hpp"

namespace goods_supplier {
struct PostGoodsView;
[[nodiscard]] auto calcSupply(const PostGoodsView& view) -> double;

[[nodiscard]] auto calcMarkup(
    const PostGoodsView& view, const double epsilonMarkup = config::goods_supplier::epsilonMarkup
) -> double;

[[nodiscard]] auto judgePrice(
    const double markup,
    const double totalCost,
    const double epsilonPrice = config::goods_supplier::epsilonPrice
) -> double;

[[nodiscard]] auto calcTotalDemand(const tbb::concurrent_vector<world::GoodsRequest>& requestBox)
    -> double;

[[nodiscard]] auto performRationedTrade(
    const double supply, tbb::concurrent_vector<world::GoodsRequest>& requestBox
) -> double;

[[nodiscard]] auto performFullTrade(tbb::concurrent_vector<world::GoodsRequest>& requestBox)
    -> double;

struct UpdateLogView {
    UpdateLogView(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto getTargetInvRatio() const -> double {
        return comp_.parameter_.targetInventoryRatio_;
    }

    [[nodiscard]] auto inventory() const -> double { return comp_.production_.inventory_; }
    [[nodiscard]] auto supply() const -> double { return comp_.plan_.supply_; }
    [[nodiscard]] auto price() const -> double { return comp_.plan_.price_; }

    void updateLog(const double price, const double sales, const bool isSold) {
        auto& log  = comp_.log_;
        log.price_ = price, log.sales_ = sales, log.isSold_ = isSold;
        auto& plan  = comp_.plan_;
        log.markup_ = plan.markup_, log.supply_ = plan.supply_;
    }

  private:
    Component& comp_;
};

void updateLog(UpdateLogView view, const double salesAmount);
}  // namespace goods_supplier