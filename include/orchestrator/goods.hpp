#pragma once

#include <tbb/concurrent_vector.h>

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier/goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "core/values/common.hpp"

namespace goods {
void product(labor::supplier::LaborSupplier& laborSupplier) { laborSupplier.product(); }

void postGoods(
    supplier::GoodsSupplier&                   goodsSupplier,
    const labor::demander::LaborDemander&      laborDemander,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
) {
    goodsSupplier.post(laborDemander.sumWage(), entryBox);
}

void purchase(
    const hhold_finance::Component&            finance,
    demander::GoodsDemander&                   goodsDemander,
    const labor::supplier::LaborSupplier&      laborSupplier,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const Step                                 step
) {
    goodsDemander.request(finance.asset() + laborSupplier.wage(), step, entryBox);
}

void trade(supplier::GoodsSupplier& goodsSupplier) { goodsSupplier.trade(); }

void afterTrade(demander::GoodsDemander& goodsDemander) { goodsDemander.afterTrade(); }

void endStep(
    firm_finance::Component& finance,
    supplier::GoodsSupplier& goodsSupplier,
    world::CensusDropBox&    dropBox
) {
    finance.assetPlus(goodsSupplier.sales());
    goodsSupplier.endStep(dropBox);
}

void endStep(hhold_finance::Component& finance, demander::GoodsDemander& goodsDemander) {
    finance.assetPlus(-goodsDemander.purchase());
    goodsDemander.endStep();
}
}  // namespace goods