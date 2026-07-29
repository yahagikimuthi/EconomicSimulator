#pragma once

#include <tbb/concurrent_vector.h>
#include <components/labor_demander.hpp>

#include "core/forward.hpp"

namespace labor {
void adjustWorkforce(
    const agent_index::Component&                indexComp,
    const goods_supplier::GoodsSupplier&         goodsSupplier,
    labor_demander::LaborDemander&               laborDemander,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
);

void jobEntry(
    const agent_index::Component&                indexComp,
    labor_supplier::Component&                   laborSupplier,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
);

void offer(labor_demander::LaborDemander& laborDemander);

void acceptOffer(labor_supplier::Component& laborSupplier);

void registerMember(
    goods_supplier::GoodsSupplier& goodsSupplier, labor_demander::LaborDemander& laborDemander
);

void recordRosterEntry(labor_supplier::Component& laborSuppler);

void acceptResignation(labor_demander::LaborDemander& laborDemander);

void endStep(
    firm_finance::Component&       financeComp,
    labor_demander::LaborDemander& laborDemander,
    world::CensusDropBox&          dropBox
);
void endStep(
    hhold_finance::Component&  financeComp,
    labor_supplier::Component& laborSupplier,
    world::CensusDropBox&      dropBox
);
}  // namespace labor