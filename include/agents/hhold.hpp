#pragma once

#include "components/consumer_goods_demander.hpp"
#include "components/finance/finance.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "values/common.hpp"
#include "world/labor.hpp"

namespace abm {
struct HHold final {
    finance::Finance      finance;
    LaborSupplier         laborSupplier;
    ConsumerGoodsDemander consumerGoodsDemander;
    const AgentID         id;
};
}  // namespace abm

namespace abm::hhold {
void laborEntry(const AgentID id, LaborSupplier& laborSupplier, LaborMarket& market) noexcept {
    laborSupplier.entry(id, market);
}

void acceptOffer(LaborSupplier& laborSupplier) noexcept { laborSupplier.accept(); }

void recordRosterEntry(LaborSupplier& laborSupplier) noexcept { laborSupplier.recordRosterEntry(); }

void work(LaborSupplier& laborSupplier) noexcept { laborSupplier.product(); }
}  // namespace abm::hhold