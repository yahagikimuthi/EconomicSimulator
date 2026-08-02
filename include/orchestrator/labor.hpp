#pragma once

#include <tbb/concurrent_vector.h>

#include "core/forward.hpp"

namespace labor {
void adjustWorkforce(
    const agent_index::Component&                index,
    const goods::supplier::GoodsSupplier&        goodsSupplier,
    demander::LaborDemander&                     laborDemander,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
);

void jobEntry(
    const agent_index::Component&                index,
    supplier::LaborSupplier&                     laborSupplier,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
);

void offer(demander::LaborDemander& laborDemander);

void acceptOffer(supplier::LaborSupplier& laborSupplier);

void registerMember(
    goods::supplier::GoodsSupplier& goodsSupplier, demander::LaborDemander& laborDemander
);

void recordRosterEntry(supplier::LaborSupplier& laborSuppler);

void acceptResignation(demander::LaborDemander& laborDemander);

void endStep(
    firm_finance::Component& finance,
    demander::LaborDemander& laborDemander,
    world::CensusDropBox&    dropBox
);
void endStep(
    hhold_finance::Component& finance,
    supplier::LaborSupplier&  laborSupplier,
    world::CensusDropBox&     dropBox
);
}  // namespace labor