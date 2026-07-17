#pragma once

#include <tbb/concurrent_vector.h>

#include "components/goods_supplier.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"

namespace goods_supplier {

struct [[nodiscard]] CalcDesiredEmployView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto firmProductivity() const -> double { return comp_.production_.firmProductPower_; }
};

[[nodiscard]] auto calcDesiredEmploy(CalcDesiredEmployView view) -> int;
struct [[nodiscard]] PostGoodsView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    void myEntry(tbb::concurrent_vector<world::GoodsEntry>::iterator it) {  // NOLINT
        comp_.posting_.myEntry_ = &*it;
    }
    void plan(const double price, const double supply, const double markup) {
        auto& plan  = comp_.plan_;
        plan.price_ = price, plan.markup_ = markup, plan.supply_ = supply;
        comp_.salesLedger.inventory_ = supply;
    }
    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }
};

void postGoods(
    PostGoodsView view, const double totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox
);

struct [[nodiscard]] TradeView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto myEntry() -> world::GoodsEntry& { return *comp_.posting_.myEntry_; }
    void inventoryMinus(double minus) { comp_.salesLedger.inventory_ -= minus; }
    void salesPlus(const double plus) { comp_.salesLedger.currentSales_ += plus; }
    void totalDemandPlus(const double plus) { comp_.salesLedger.totalDemand_ += plus; }
    auto isPosting() const -> bool { return comp_.posting_.isPosting_; }
    auto rng() -> pcg32& { return comp_.rng_; }
};

void trade(TradeView view);
void logging(world::CensusDropBox& dropBox, const Component& comp);
void reset(Component& comp);
}  // namespace goods_supplier