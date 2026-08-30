#pragma once

#include "components/capital_demander.hpp"
#include "components/capital_supplier/capital_supplier.hpp"
#include "components/consumer_goods_supplier/consumer_goods_supplier.hpp"
#include "components/finance/finance.hpp"
#include "components/government.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "world/capital.hpp"
#include "world/common.hpp"

namespace abm::capital {
void product(LaborSupplier& laborSupplier, const EMarket phase) noexcept {
    laborSupplier.product(phase);
}

void postGoods(
    const AgentID&       id,
    CapitalSupplier&     goodsSupplier,
    const LaborDemander& laborDemander,
    CapitalMarket&       market
) noexcept {
    goodsSupplier.post(id, laborDemander.sumWage(), market);
}

void purchase(
    const AgentID&         id,
    const FirmFinance&     finance,
    CapitalDemander&       capitalDemander,
    ConsumerGoodsSupplier& consumerGoodsSupplier,
    CapitalMarket&         market
) noexcept {
    const auto desired = consumerGoodsSupplier.requiresCapital();
    capitalDemander.request(id, finance.asset(), desired, market);
}

void purchase(
    const AgentID&     id,
    const FirmFinance& finance,
    CapitalDemander&   capitalDemander,
    CapitalSupplier&   capitalSupplier,
    CapitalMarket&     market
) noexcept {
    const auto desired = capitalSupplier.requiresCapital();
    capitalDemander.request(id, finance.asset(), desired, market);
}

void trade(CapitalSupplier& goodsSupplier) noexcept { goodsSupplier.trade(); }

void afterTrade(CapitalDemander& goodsDemander) noexcept { goodsDemander.afterTrade(); }

void endStep(
    FirmFinance&     finance,
    CapitalSupplier& capitalSupplier,
    CapitalDemander& capitalDemander,
    Government&      government,
    CensusDropBox&   dropBox
) noexcept {
    capitalSupplier.endStep(
        [&](const Money sales) noexcept -> void {
            const auto salesAfterTax = government.collectSalesTax(sales);
            finance.assetPlus(salesAfterTax);
        },
        dropBox
    );
    capitalDemander.endStep([&](const demander::TradeResult& result) noexcept -> void {
        finance.assetPlus(-result.purchased);
        capitalSupplier.addCapitalEquip(result.tradeAmount);
    });
}

void endStep(
    FirmFinance&           finance,
    ConsumerGoodsSupplier& consumerGoodsSupplier,
    CapitalDemander&       capitalDemander
) noexcept {
    capitalDemander.endStep([&](const demander::TradeResult& result) noexcept -> void {
        finance.assetPlus(-result.purchased);
        consumerGoodsSupplier.addCapitalEquip(result.tradeAmount);
    });
}

}  // namespace abm::capital