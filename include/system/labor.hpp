#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/finance/others_finance.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "values/common.hpp"
#include "world/labor.hpp"

namespace abm::labor {
inline void adjustWorkforce(const AgentID id, LaborDemander& demander, Market& market) noexcept {
    demander.adjustWorkforce(id, market);
}

inline void entry(const AgentID id, LaborSupplier& supplier, Market& market) noexcept {
    supplier.entry(id, market);
}

inline void offer(LaborDemander& demander) noexcept { demander.offer(); }

inline void accept(LaborSupplier& supplier) noexcept { supplier.accept(); }

inline void endRecruiting(LaborDemander& demander, CapitalSupplier& capitalSupplier) noexcept {
    demander.endRecruiting(capitalSupplier.workspace());
}

inline void endRecruiting(LaborDemander& demander, GoodsSupplier& goodsSupplier) noexcept {
    demander.endRecruiting(goodsSupplier.workspace());
}

inline void recordRosterEntry(LaborSupplier& supplier) noexcept { supplier.recordRosterEntry(); }

inline void payWage(FirmFinance& finance, LaborDemander& demander) noexcept {
    demander.payWage(finance.makeWithdrawFn(FirmFinance::AccountItem::PersonalCost));
}

inline void work(HHoldFinance& finance, LaborSupplier& supplier) noexcept {
    supplier.work(finance.makeDepositFn());
}
}  // namespace abm::labor