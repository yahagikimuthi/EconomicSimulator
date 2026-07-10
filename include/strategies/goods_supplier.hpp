#pragma once

#include <tbb/concurrent_vector.h>

#include "components/goods_supplier.hpp"
#include "core/forward.hpp"

namespace goods_supplier {
struct PostGoodsView {
    explicit PostGoodsView(Component& comp) : comp_{comp} {}
    void myEntry(const tbb::concurrent_vector<world::GoodsEntry>::iterator it) {  // NOLINT
        comp_.posting_.myEntry_ = &*it;
    }

    void plan(const double price, const double supply, const double markup) {
        auto& plan  = comp_.plan_;
        plan.price_ = price, plan.markup_ = markup, plan.supply_ = supply;
        comp_.salesLedger.inventory_ = supply;
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
    friend struct CalcSupplyView;
    friend struct CalcMarkupView;
};

void postGoods(
    PostGoodsView view, const double totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox
);

struct TradeView {
    explicit TradeView(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto myEntry() const -> world::GoodsEntry& { return *comp_.posting_.myEntry_; }

    void inventoryMinus(double inventoryMinus) { comp_.salesLedger.inventory_ -= inventoryMinus; }
    void salesPlus(const double salesPlus) { comp_.salesLedger.currentSales += salesPlus; }

    [[nodiscard]] auto targetInvRatio() const -> double {
        return comp_.parameter_.targetInventoryRatio_;
    }
    [[nodiscard]] auto inventory() const -> double { return comp_.salesLedger.inventory_; }
    [[nodiscard]] auto price() const -> double { return comp_.plan_.price_; }

  private:
    Component& comp_;
};

void trade(TradeView view);
void logging(world::CensusDropBox& dropBox, const Component& comp);
void reset(Component& comp);
}  // namespace goods_supplier