#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/finance/others_finance.hpp"
#include "components/goods_demander.hpp"
#include "values/common.hpp"
#include "world/base_goods.hpp"

namespace abm::goods {
inline void entry(const AgentID id, GoodsSupplier& supplier, Market& market) noexcept {
    supplier.post(id, market);
}

inline void request(
    const AgentID id, HHoldFinance& finance, GoodsDemander& demander, Market& market
) noexcept {
    demander.request(id, finance.makeWithdrawFn(), market);
}

inline void trade(FirmFinance& finance, GoodsSupplier& supplier) noexcept {
    supplier.trade(finance.makeDepositFn(FirmFinance::AccountItem::Sales));
}

inline void afterTrade(HHoldFinance& finance, GoodsDemander& demander) noexcept {
    demander.afterTrade(finance.makeDepositFn());
}

inline void afterTrade(GoodsSupplier& supplier) noexcept { supplier.endTrading(); }
}  // namespace abm::goods