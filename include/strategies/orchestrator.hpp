#pragma once

#include <tbb/concurrent_vector.h>

#include "core/forward.hpp"

namespace orchestrator::labor {
void AdjustWorkforce(
    const agent_index::Component&                indexComp,
    goods_supplier::Component&                   goodsSupplier,
    labor_demander::Component&                   laborDemander,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    std::vector<world::CompanyBoard>&            companyBoards
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

void recordRosterEntry(labor_supplier::Component& laborSuppler);

void acceptResignation(labor_demander::Component laborDemander);

void endStep(
    firm_finance::Component&   financeComp,
    labor_demander::Component& laborDemander,
    world::CensusDropBox&      dropBox
);
void endStep(
    hhold_finance::Component&  financeComp,
    labor_supplier::Component& laborSupplier,
    world::CensusDropBox&      dropBox
);
}  // namespace orchestrator::labor

namespace orchestrator::goods {

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
}  // namespace orchestrator::goods