#pragma once

#include "components/common.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/government.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "components/production_goods_supplier/production_goods_supplier.hpp"
#include "core/values/labor.hpp"
#include "world/common.hpp"
#include "world/labor.hpp"

namespace abm::labor {
void adjustWorkforce(
    const AgentID&         id,
    ConsumerGoodsSupplier& goodsSupplier,
    LaborDemander&         laborDemander,
    LaborMarket&           laborMarket
) noexcept {
    const auto desiredEmploy = goodsSupplier.calcDesiredEmploy(laborDemander.employeeCnt());
    laborDemander.adjustWorkforce(id, desiredEmploy, goodsSupplier.salesForecast(), laborMarket);
}

void adjustWorkforce(
    const AgentID&           id,
    ProductionGoodsSupplier& goodsSupplier,
    LaborDemander&           laborDemander,
    LaborMarket&             laborMarket
) noexcept {
    const auto desiredEmploy = goodsSupplier.calcDesiredEmploy(laborDemander.employeeCnt());
    laborDemander.adjustWorkforce(id, desiredEmploy, goodsSupplier.salesForecast(), laborMarket);
}

void jobEntry(const AgentID& id, LaborSupplier& laborSupplier, LaborMarket& laborMarket) noexcept {
    laborSupplier.entry(id, laborMarket);
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
    laborDemander.endStep(
        [&](const Money totalCost) -> void { finance.assetPlus(-totalCost); }, dropBox
    );
}

void endStep(
    HHoldFinance&  finance,
    LaborSupplier& laborSupplier,
    Government&    government,
    CensusDropBox& dropBox
) noexcept {
    laborSupplier.endStep(
        [&](const Money wage) -> void {
            const auto incomeAfterTax = government.collectIncomeTax(wage);
            finance.assetPlus(incomeAfterTax);
        },
        dropBox
    );
}
}  // namespace abm::labor