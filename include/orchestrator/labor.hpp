#pragma once

#include "components/common.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_supplier/production_goods_supplier.hpp"
#include "core/values/labor.hpp"
#include "world/common.hpp"
#include "world/labor.hpp"

namespace abm::labor {
void adjustWorkforce(
    const AgentIndex&      index,
    ConsumerGoodsSupplier& goodsSupplier,
    LaborDemander&         laborDemander,
    LaborMarket&           laborMarket
) noexcept {
    const auto desiredEmploy = goodsSupplier.calcDesiredEmploy(laborDemander.employeeCnt());
    laborDemander.adjustWorkforce(
        index.id(), desiredEmploy, goodsSupplier.salesForecast(), laborMarket
    );
}

void adjustWorkforce(
    const AgentIndex&        index,
    ProductionGoodsSupplier& goodsSupplier,
    LaborDemander&           laborDemander,
    LaborMarket&             laborMarket
) noexcept {
    const auto desiredEmploy = goodsSupplier.calcDesiredEmploy(laborDemander.employeeCnt());
    laborDemander.adjustWorkforce(
        index.id(), desiredEmploy, goodsSupplier.salesForecast(), laborMarket
    );
}

void jobEntry(
    const AgentIndex& index, LaborSupplier& laborSupplier, LaborMarket& laborMarket
) noexcept {
    laborSupplier.entry(index.id(), laborMarket);
}

void offer(LaborDemander& laborDemander) noexcept { laborDemander.offer(); }

void acceptOffer(LaborSupplier& laborSupplier) noexcept { laborSupplier.accept(); }

void registerMember(ConsumerGoodsSupplier& goodsSupplier, LaborDemander& laborDemander) noexcept {
    laborDemander.registerMember(goodsSupplier.workspace());
}

void registerMember(ProductionGoodsSupplier& goodsSupplier, LaborDemander& laborDemander) noexcept {
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