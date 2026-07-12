#pragma once

#include <tbb/concurrent_vector.h>

#include "components/goods_supplier.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"

namespace goods_supplier {
struct [[nodiscard]] PostGoodsView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    void myEntry(const tbb::concurrent_vector<world::GoodsEntry>::iterator it) {  // NOLINT
        comp_.posting_.myEntry_ = &*it;
    }
    void plan(const double price, const double supply, const double markup) {
        auto& plan  = comp_.plan_;
        plan.price_ = price, plan.markup_ = markup, plan.supply_ = supply;
        comp_.salesLedger.inventory_ = supply;
    }
    auto firmProductPower() const -> double { return comp_.production_.firmProductPower_; }
    auto sumEmployeeProductPower() const -> double {
        return comp_.production_.sumEmployeeProductPower_;
    }
    auto inventory() const -> double { return comp_.production_.inventory_; }
    auto markupAdjustVol() const -> double { return comp_.parameter_.markupAdjustmentVolatility_; }
    auto lastMarkup() const -> double { return comp_.log_.markup_; }
    auto isSold() const -> bool { return comp_.log_.isSold_; }
    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }

    friend struct CalcSupplyView;
    friend struct CalcMarkupView;
    friend struct JudgePriceView;
};

void postGoods(
    PostGoodsView                              view,
    const double                               totalCost,
    const double                               employeeCnt,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
);

struct [[nodiscard]] TradeView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto myEntry() const -> world::GoodsEntry& { return *comp_.posting_.myEntry_; }
    void inventoryMinus(double inventoryMinus) { comp_.salesLedger.inventory_ -= inventoryMinus; }
    void salesPlus(const double salesPlus) { comp_.salesLedger.currentSales += salesPlus; }
    auto targetInvRatio() const -> double { return comp_.parameter_.targetInventoryRatio_; }
    auto inventory() const -> double { return comp_.salesLedger.inventory_; }
    auto price() const -> double { return comp_.plan_.price_; }
    auto isPosting() const -> bool { return comp_.posting_.isPosting_; }
};

void trade(TradeView view);
void logging(world::CensusDropBox& dropBox, const Component& comp);
void reset(Component& comp);
}  // namespace goods_supplier