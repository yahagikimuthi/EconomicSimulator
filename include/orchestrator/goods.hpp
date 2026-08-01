#pragma once

#include <tbb/concurrent_vector.h>

#include "core/forward.hpp"
#include "core/values/common.hpp"

namespace goods {
void product(labor_supplier::LaborSupplier& laborSupplier);

void postGoods(
    goods_supplier::GoodsSupplier&             goodsSupplier,
    const labor_demander::LaborDemander&       laborDemander,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
);

void purchase(
    const hhold_finance::Component&            finance,
    goods_demander::GoodsDemander&             goodsDemander,
    const labor_supplier::LaborSupplier&       laborSupplier,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const Step                                 step
);

void trade(goods_supplier::GoodsSupplier& goodsSupplier);

void afterTrade(goods_demander::GoodsDemander& goodsDemander);

void endStep(
    firm_finance::Component&       finance,
    goods_supplier::GoodsSupplier& goodsSupplier,
    world::CensusDropBox&          dropBox
);
void endStep(hhold_finance::Component& finance, goods_demander::GoodsDemander& goodsDemander);
}  // namespace goods