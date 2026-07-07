#pragma once

#include <tbb/concurrent_vector.h>

#include "components/goods_supplier.hpp"
#include "core/forward.hpp"

namespace goods_supplier {

struct IsSoldView {
    IsSoldView(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto inventory() const -> double { return comp_.production_.inventory_; }
    [[nodiscard]] auto targetInvRatio() const -> double {
        return comp_.parameter_.targetInventoryRatio_;
    }
    [[nodiscard]] auto lastSupply() const -> double { return comp_.log_.supply_; }

  private:
    Component& comp_;
};

[[nodiscard]] auto isSold(const IsSoldView& view) -> bool;

struct PostGoodsView {
    PostGoodsView(Component& comp) : comp_{comp} {}
    void setMyEntry(const tbb::concurrent_vector<world::GoodsEntry>::iterator it) {  // NOLINT
        comp_.posting_.myEntry_ = it;
    }

    void setPlan(const double price, const double supply, const double markup) {
        auto& plan  = comp_.plan_;
        plan.price_ = price, plan.markup_ = markup, plan.supply_ = supply;
    }

    [[nodiscard]] auto firmProductPower() const -> double {
        return comp_.production_.firmProductPower_;
    }
    [[nodiscard]] auto sumEmployeeProductPower() const -> double {
        return comp_.production_.sumEmployeeProductPower_;
    }
    [[nodiscard]] auto inventory() const -> double { return comp_.production_.inventory_; }
    [[nodiscard]] auto markupAdjustVol() const -> double {
        return comp_.parameter_.markupAdjustmentVolatility_;
    }
    [[nodiscard]] auto lastMarkup() const -> double { return comp_.log_.markup_; }
    [[nodiscard]] auto isSold() const -> bool { return comp_.log_.isSold_; }

  private:
    Component& comp_;
};

void postGoods(
    PostGoodsView view, const double totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox
);

struct TradeView {
    TradeView(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto getMyEntry() const -> tbb::concurrent_vector<world::GoodsEntry>::iterator {
        return comp_.posting_.myEntry_;
    }

    [[nodiscard]] auto supply() const -> double { return comp_.plan_.supply_; }

    void               setInventory(double inventory) { comp_.production_.inventory_ = inventory; }
    [[nodiscard]] auto getTargetInvRatio() const -> double {
        return comp_.parameter_.targetInventoryRatio_;
    }
    [[nodiscard]] auto inventory() const -> double { return comp_.production_.inventory_; }
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

void trade(TradeView view);
}  // namespace goods_supplier