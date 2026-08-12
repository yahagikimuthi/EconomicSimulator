#pragma once

#include <tbb/concurrent_vector.h>

#include "components/common.hpp"
#include "components/consumer_goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace abm::labor {
void adjustWorkforce(
    const AgentIndex&                     index,
    const ConsumerGoodsSupplier&          goodsSupplier,
    LaborDemander&                        laborDemander,
    tbb::concurrent_vector<LaborRequest>& requestBox
) {
    const HeadCount desiredEmploy{goodsSupplier.calcDesiredEmploy(laborDemander.employeeCnt())};
    if (desiredEmploy > HeadCount{0.0}) {
        laborDemander.post(index.id(), desiredEmploy, requestBox);
    } else if (desiredEmploy < HeadCount{0.0}) {
        laborDemander.layOffs(-desiredEmploy);
    }
}

void jobEntry(
    const AgentIndex&                     index,
    LaborSupplier&                        laborSupplier,
    tbb::concurrent_vector<LaborRequest>& requestBox
) {
    laborSupplier.entry(index.id(), requestBox);
}

void offer(LaborDemander& laborDemander) { laborDemander.offer(); }

void acceptOffer(LaborSupplier& laborSupplier) { laborSupplier.accept(); }

void registerMember(ConsumerGoodsSupplier& goodsSupplier, LaborDemander& laborDemander) {
    laborDemander.registerMember(goodsSupplier.workspace());
}

void recordRosterEntry(LaborSupplier& laborSuppler) { laborSuppler.recordRosterEntry(); }

void acceptResignation(LaborDemander& laborDemander) { laborDemander.acceptResignation(); }

void endStep(FirmFinance& finance, LaborDemander& laborDemander, CensusDropBox& dropBox) {
    laborDemander.endStep(dropBox);
    finance.assetPlus(-laborDemander.sumWage());
}
void endStep(HHoldFinance& finance, LaborSupplier& laborSupplier, CensusDropBox& dropBox) {
    finance.assetPlus(laborSupplier.wage());
    laborSupplier.endStep(dropBox);
}
}  // namespace abm::labor