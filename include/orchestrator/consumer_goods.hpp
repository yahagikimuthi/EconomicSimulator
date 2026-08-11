#pragma once

#include <tbb/concurrent_vector.h>

#include "components/common.hpp"
#include "components/consumer_goods_demander.hpp"
#include "components/consumer_goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "core/values/common.hpp"
#include "world/message.hpp"

namespace consumer_goods {
void product(labor::supplier::LaborSupplier& laborSupplier, const MarketPhase phase) {
    laborSupplier.product(phase);
}

void postGoods(
    supplier::ConsumerGoodsSupplier&            goodsSupplier,
    const labor::demander::LaborDemander&       laborDemander,
    tbb::concurrent_vector<ConsumerGoodsEntry>& entryBox
) {
    goodsSupplier.post(laborDemander.sumWage(), entryBox);
}

void purchase(
    const hhold_finance::Component&             finance,
    demander::ConsumerGoodsDemander&            goodsDemander,
    const labor::supplier::LaborSupplier&       laborSupplier,
    tbb::concurrent_vector<ConsumerGoodsEntry>& entryBox,
    const Step                                  step
) {
    goodsDemander.request(finance.asset() + laborSupplier.wage(), step, entryBox);
}

void trade(supplier::ConsumerGoodsSupplier& goodsSupplier) { goodsSupplier.trade(); }

void afterTrade(demander::ConsumerGoodsDemander& goodsDemander) { goodsDemander.afterTrade(); }

void endStep(
    firm_finance::Component&         finance,
    supplier::ConsumerGoodsSupplier& goodsSupplier,
    CensusDropBox&                   dropBox
) {
    finance.assetPlus(goodsSupplier.sales());
    goodsSupplier.endStep(dropBox);
}

void endStep(hhold_finance::Component& finance, demander::ConsumerGoodsDemander& goodsDemander) {
    finance.assetPlus(-goodsDemander.purchase());
    goodsDemander.endStep();
}
}  // namespace consumer_goods