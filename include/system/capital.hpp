#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/capital_demander.hpp"
#include "components/finance/firm_finance.hpp"
#include "values/common.hpp"
#include "world/base_goods.hpp"

namespace abm::capital {
inline void entry(const AgentID id, CapitalSupplier& supplier, Market& market) noexcept {
    supplier.post(id, market);
}

inline void request(
    const AgentID id, FirmFinance& finance, CapitalDemander& demander, Market& market
) noexcept {
    demander.request(
        id, finance.makeWithdrawFn(FirmFinance::AccountItem::CapitalGoodsCost), market
    );
}

inline void trade(FirmFinance& finance, CapitalSupplier& supplier) noexcept {
    supplier.trade(finance.makeDepositFn(FirmFinance::AccountItem::Sales));
}

inline void afterTrade(
    FirmFinance& finance, CapitalDemander& demander, CapitalSupplier& capitalSupplier
) noexcept {
    demander.afterTrade(
        finance.makeDepositFn(FirmFinance::AccountItem::CapitalGoodsCost),
        capitalSupplier.makeAddCapitalFn()
    );
}

inline void afterTrade(
    FirmFinance& finance, CapitalDemander& demander, GoodsSupplier& goodsSupplier
) noexcept {
    demander.afterTrade(
        finance.makeDepositFn(FirmFinance::AccountItem::CapitalGoodsCost),
        goodsSupplier.makeAddCapitalFn()
    );
}
}  // namespace abm::capital