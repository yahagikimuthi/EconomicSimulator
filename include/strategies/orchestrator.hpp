#pragma once

#include "components/common.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander.hpp"

namespace orchestrator {
void postLaborRequest(
    const agent_index::Component&                indexComp,
    goods_supplier::Component&                   goodsSupplier,
    labor_demander::Component&                   laborDemander,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
);

void offer(labor_demander::Component& laborDemander);

void registerMember(
    goods_supplier::Component& goodsSupplier, labor_demander::Component& laborDemander
);
}  // namespace orchestrator