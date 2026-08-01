#pragma once

#include <tbb/concurrent_vector.h>

#include "core/forward.hpp"

namespace labor {
void adjustWorkforce(
    const agent_index::Component&                index,
    const goods_supplier::GoodsSupplier&         goodsSupplier,
    labor_demander::LaborDemander&               laborDemander,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
);

void jobEntry(
    const agent_index::Component&                index,
    labor_supplier::LaborSupplier&               laborSupplier,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
);

void offer(labor_demander::LaborDemander& laborDemander);

void acceptOffer(labor_supplier::LaborSupplier& laborSupplier);

void registerMember(
    goods_supplier::GoodsSupplier& goodsSupplier, labor_demander::LaborDemander& laborDemander
);

void recordRosterEntry(labor_supplier::LaborSupplier& laborSuppler);

void acceptResignation(labor_demander::LaborDemander& laborDemander);

void endStep(
    firm_finance::Component&       finance,
    labor_demander::LaborDemander& laborDemander,
    world::CensusDropBox&          dropBox
);
void endStep(
    hhold_finance::Component&      finance,
    labor_supplier::LaborSupplier& laborSupplier,
    world::CensusDropBox&          dropBox
);
}  // namespace labor