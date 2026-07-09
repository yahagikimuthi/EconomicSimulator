#pragma once

#include <tbb/concurrent_vector.h>

#include "core/forward.hpp"

namespace orchestrator {
void postLaborRequest(
    const agent_index::Component&                indexComp,
    goods_supplier::Component&                   goodsSupplier,
    labor_demander::Component&                   laborDemander,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
);

void jobEntry(
    const agent_index::Component&                indexComp,
    labor_supplier::Component&                   laborSupplier,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
);

void offer(labor_demander::Component& laborDemander);

void acceptOffer(labor_supplier::Component& laborSupplier);

void registerMember(
    goods_supplier::Component& goodsSupplier, labor_demander::Component& laborDemander
);

void postGoods(
    goods_supplier::Component&                 goodsSupplier,
    labor_demander::Component&                 laborDemander,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
);

void purchase(
    hhold_finance::Component&                  financeComponent,
    goods_demander::Component&                 goodsDemander,
    labor_supplier::Component&                 laborSupplier,
    tbb::concurrent_vector<world::GoodsEntry>& entryBox
);

void trade(goods_supplier::Component& goodsSupplier);
}  // namespace orchestrator