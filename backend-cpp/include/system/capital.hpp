#pragma once

#include "components/base_goods_supplier/base_goods_supplier.hpp"
#include "components/capital_demander.hpp"
#include "components/finance/firm_finance.hpp"
#include "components/government.hpp"
#include "values/common.hpp"
#include "world/base_goods.hpp"
#include "world/drop_box.hpp"

namespace abm::capital {
inline void entry(const AgentID id, CapitalSupplier& supplier, Market& market) noexcept {
    supplier.post(id, market);
}

inline void request(
    const AgentID id, FirmFinance& finance, CapitalDemander& demander, Market& market
) noexcept {
    demander.request(id, finance.makeWithdrawFn(), market);
}

inline void trade(
    FirmFinance& finance, CapitalSupplier& supplier, Government& government
) noexcept {
    supplier.trade([&](const Money sales) noexcept -> void {
        const auto afterTax = government.paySalesTax(sales);
        finance.deposit(afterTax);
    });
}

inline void afterTrade(
    FirmFinance& finance, CapitalDemander& demander, CapitalSupplier& capitalSupplier
) noexcept {
    demander.afterTrade(finance.makeDepositFn(), capitalSupplier.makeAddCapitalFn());
}

inline void afterTrade(
    FirmFinance& finance, CapitalDemander& demander, GoodsSupplier& goodsSupplier
) noexcept {
    demander.afterTrade(finance.makeDepositFn(), goodsSupplier.makeAddCapitalFn());
}

inline void logging(CapitalDropBox& dropBox, CapitalSupplier& supplier) noexcept {
    supplier.logging(dropBox);
}
}  // namespace abm::capital