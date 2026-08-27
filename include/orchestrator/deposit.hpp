#pragma once

#include "components/finance/deposit_demander.hpp"
#include "components/finance/deposit_supplier.hpp"
#include "components/finance/finance.hpp"
#include "core/values/common.hpp"
#include "world/deposit.hpp"

namespace abm::deposit {
void postRequest(
    const AgentID id, DepositDemander& depositDemander, DepositMarket& market
) noexcept {
    depositDemander.postRequest(id, market);
}

void entry(const AgentID id, DepositSupplier& depositSupplier, DepositMarket& market) noexcept {
    depositSupplier.requestAccountOpening(id, market);
}

void accountOpening(DepositDemander& depositDemander) noexcept { depositDemander.accountOpening(); }

void registerAccount(DepositSupplier& depositSupplier) noexcept {
    depositSupplier.registerAccount();
}

void endStep(FirmFinance& finance, DepositDemander& depositDemander) noexcept {
    depositDemander.endStep([&](const Money minus) noexcept -> void { finance.assetPlus(-minus); });
}

void endStep(DepositSupplier& depositSupplier) noexcept { depositSupplier.endStep(); }
}  // namespace abm::deposit