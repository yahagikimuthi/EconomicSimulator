#pragma once

#include <tbb/concurrent_vector.h>

#include "core/forward.hpp"
#include "core/values/common.hpp"

namespace goods {
void product(labor::supplier::LaborSupplier& laborSupplier);

void postGoods(
    supplier::GoodsSupplier&                   goodsSupplier,
    const labor::demander::LaborDemander&      laborDemander,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
);

void purchase(
    const hhold_finance::Component&            finance,
    demander::GoodsDemander&                   goodsDemander,
    const labor::supplier::LaborSupplier&      laborSupplier,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const Step                                 step
);

void trade(supplier::GoodsSupplier& goodsSupplier);

void afterTrade(demander::GoodsDemander& goodsDemander);

void endStep(
    firm_finance::Component& finance,
    supplier::GoodsSupplier& goodsSupplier,
    world::CensusDropBox&    dropBox
);
void endStep(hhold_finance::Component& finance, demander::GoodsDemander& goodsDemander);
}  // namespace goods