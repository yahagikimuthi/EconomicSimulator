#pragma once

#include <tbb/concurrent_vector.h>

#include "components/common.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander/concepts.hpp"
#include "components/labor_supplier.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor {
template <demander::ILaborDemander ILaborDemander>
void adjustWorkforce(
    const agent_index::Component&                index,
    const goods::supplier::GoodsSupplier&        goodsSupplier,
    ILaborDemander&                              laborDemander,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    const HeadCount desiredEmploy{goodsSupplier.calcDesiredEmploy(laborDemander.employeeCnt())};
    if (desiredEmploy > HeadCount{0.0}) {
        laborDemander.post(index.id(), desiredEmploy, requestBox);
    } else if (desiredEmploy < HeadCount{0.0}) {
        laborDemander.layOffs(-desiredEmploy);
    }
}

void jobEntry(
    const agent_index::Component&                index,
    supplier::LaborSupplier&                     laborSupplier,
    tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    laborSupplier.entry(index.id(), requestBox);
}

template <demander::ILaborDemander ILaborDemander>
void offer(ILaborDemander& laborDemander) {
    laborDemander.offer();
}

void acceptOffer(supplier::LaborSupplier& laborSupplier) { laborSupplier.accept(); }

template <demander::ILaborDemander ILaborDemander>
void registerMember(goods::supplier::GoodsSupplier& goodsSupplier, ILaborDemander& laborDemander) {
    laborDemander.registerMember(goodsSupplier.workspace());
}

void recordRosterEntry(supplier::LaborSupplier& laborSuppler) { laborSuppler.recordRosterEntry(); }

template <demander::ILaborDemander ILaborDemander>
void acceptResignation(ILaborDemander& laborDemander) {
    laborDemander.acceptResignation();
}

template <demander::ILaborDemander ILaborDemander>
void endStep(
    firm_finance::Component& finance, ILaborDemander& laborDemander, world::CensusDropBox& dropBox
) {
    laborDemander.endStep(dropBox);
    finance.assetPlus(-laborDemander.sumWage());
}
void endStep(
    hhold_finance::Component& finance,
    supplier::LaborSupplier&  laborSupplier,
    world::CensusDropBox&     dropBox
) {
    finance.assetPlus(laborSupplier.wage());
    laborSupplier.endStep(dropBox);
}
}  // namespace labor