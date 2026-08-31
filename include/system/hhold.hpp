#pragma once

#include "components/labor_supplier/labor_supplier.hpp"
#include "values/common.hpp"
#include "world/labor.hpp"

namespace abm::hhold {
void laborEntry(const AgentID id, LaborSupplier& laborSupplier, LaborMarket& market) noexcept {
    laborSupplier.entry(id, market);
}

void acceptOffer(LaborSupplier& laborSupplier) noexcept { laborSupplier.accept(); }

void recordRosterEntry(LaborSupplier& laborSupplier) noexcept { laborSupplier.recordRosterEntry(); }

void work(LaborSupplier& laborSupplier) noexcept { laborSupplier.product(); }
}  // namespace abm::hhold