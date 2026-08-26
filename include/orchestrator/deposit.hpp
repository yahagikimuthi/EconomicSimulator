#pragma once

#include "components/deposit_demander.hpp"
#include "components/deposit_supplier.hpp"
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

void endStep(DepositDemander& depositDemander) noexcept { depositDemander.endStep(); }

void endStep(DepositSupplier& depositSupplier) noexcept { depositSupplier.endStep(); }
}  // namespace abm::deposit