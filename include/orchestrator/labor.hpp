#pragma once

#include <tbb/concurrent_vector.h>

#include "components/common.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "core/values/labor.hpp"
#include "world/common.hpp"
#include "world/labor.hpp"

namespace abm::labor {
void adjustWorkforce(
    const AgentIndex&            index,
    const ConsumerGoodsSupplier& goodsSupplier,
    LaborDemander&               laborDemander,
    LaborMarket&                 laborMarket
) noexcept {
    const auto desiredEmploy = goodsSupplier.calcDesiredEmploy(laborDemander.employeeCnt());
    if (desiredEmploy > HeadCount{0.0}) {
        laborDemander.post(index.id(), desiredEmploy, laborMarket);
    } else if (desiredEmploy < HeadCount{0.0}) {
        laborDemander.layOffs(-desiredEmploy);
    }
}

void jobEntry(
    const AgentIndex& index, LaborSupplier& laborSupplier, LaborMarket& laborMarket
) noexcept {
    laborSupplier.entry(index.id(), laborMarket.requestBox());
}

void offer(LaborDemander& laborDemander) noexcept { laborDemander.offer(); }

void acceptOffer(LaborSupplier& laborSupplier) noexcept { laborSupplier.accept(); }

void registerMember(ConsumerGoodsSupplier& goodsSupplier, LaborDemander& laborDemander) noexcept {
    laborDemander.registerMember(goodsSupplier.workspace());
}

void recordRosterEntry(LaborSupplier& laborSuppler) noexcept { laborSuppler.recordRosterEntry(); }

void acceptResignation(LaborDemander& laborDemander) noexcept { laborDemander.acceptResignation(); }

void endStep(FirmFinance& finance, LaborDemander& laborDemander, CensusDropBox& dropBox) noexcept {
    laborDemander.endStep(dropBox);
    finance.assetPlus(-laborDemander.sumWage());
}
void endStep(HHoldFinance& finance, LaborSupplier& laborSupplier, CensusDropBox& dropBox) noexcept {
    finance.assetPlus(laborSupplier.wage());
    laborSupplier.endStep(dropBox);
}
}  // namespace abm::labor