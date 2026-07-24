#pragma once

#include <tbb/concurrent_vector.h>

#include "core/forward.hpp"

namespace goods {
void postGoods(
    goods_supplier::Component&                 goodsSupplier,
    const labor_demander::Component&           laborDemander,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
);

void purchase(
    const hhold_finance::Component&            financeComponent,
    goods_demander::Component&                 goodsDemander,
    const labor_supplier::Component&           laborSupplier,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                  step
);

void trade(goods_supplier::Component& goodsSupplier);

void afterTrade(goods_demander::Component& goodsDemander);

void endStep(
    firm_finance::Component&   financeComp,
    goods_supplier::Component& goodsSupplier,
    world::CensusDropBox&      dropBox
);
void endStep(hhold_finance::Component& financeComp, goods_demander::Component& goodsDemander);
}  // namespace goods