#pragma once

#include "components/common.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_demander.hpp"
#include "components/production_goods_supplier.hpp"
#include "world/message.hpp"

namespace production_goods {
void product(labor::supplier::LaborSupplier& laborSupplier, const Market phase) {
    laborSupplier.product(phase);
}

void postGoods(
    supplier::ProductionGoodsSupplier&            goodsSupplier,
    const labor::demander::LaborDemander&         laborDemander,
    tbb::concurrent_vector<ProductionGoodsEntry>& entryBox
) {
    goodsSupplier.post(laborDemander.sumWage(), entryBox);
}

void purchase(
    const firm_finance::Component&                finance,
    demander::ProductionGoodsDemander&            goodsDemander,
    const labor::demander::LaborDemander&         laborSupplier,
    tbb::concurrent_vector<ProductionGoodsEntry>& entryBox
) {
    goodsDemander.request(finance.asset() - laborSupplier.sumWage(), entryBox);
}

void trade(supplier::ProductionGoodsSupplier& goodsSupplier) { goodsSupplier.trade(); }

void afterTrade(demander::ProductionGoodsDemander& goodsDemander) { goodsDemander.afterTrade(); }

void endStep(
    firm_finance::Component&           finance,
    supplier::ProductionGoodsSupplier& goodsSupplier,
    CensusDropBox&                     dropBox
) {
    finance.assetPlus(goodsSupplier.sales());
    goodsSupplier.endStep(dropBox);
}

void endStep(firm_finance::Component& finance, demander::ProductionGoodsDemander& goodsDemander) {
    finance.assetPlus(-goodsDemander.purchase());
    goodsDemander.endStep();
}

}  // namespace production_goods