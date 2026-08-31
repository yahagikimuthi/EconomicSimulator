#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "values/common.hpp"
#include "world/labor.hpp"

namespace abm::labor::demander {
inline void layOffs(LaborDemander& laborDemander) noexcept { laborDemander.layOffs(); }

inline void postLaborRequest(
    const AgentID id, LaborDemander& laborDemander, Market& market
) noexcept {
    laborDemander.postLaborRequest(id, market);
}

inline void offer(LaborDemander& laborDemander) noexcept { laborDemander.offer(); }

inline void registerMember(
    LaborDemander& laborDemander, BaseGoodsSupplier& goodsSupplier
) noexcept {
    laborDemander.registerMember(goodsSupplier.workspace());
}

inline void acceptResignation(LaborDemander& laborDemander) noexcept {
    laborDemander.acceptResignation();
}
}  // namespace abm::labor::demander

namespace abm::labor::supplier {
inline void work(LaborSupplier& laborSupplier) noexcept { laborSupplier.product(); }

inline void laborEntry(const AgentID id, LaborSupplier& laborSupplier, Market& market) noexcept {
    laborSupplier.entry(id, market);
}

inline void acceptOffer(LaborSupplier& laborSupplier) noexcept { laborSupplier.accept(); }

inline void recordRosterEntry(LaborSupplier& laborSupplier) noexcept {
    laborSupplier.recordRosterEntry();
}
}  // namespace abm::labor::supplier