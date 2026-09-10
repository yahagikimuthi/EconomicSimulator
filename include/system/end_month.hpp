#pragma once

#include "components/finance/firm_finance.hpp"
#include "components/finance/others_finance.hpp"
#include "components/government.hpp"
#include "components/labor_demander/labor_demander.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "world/drop_box.hpp"

namespace abm::end_month {
inline void payWage(
    FirmFinance& finance, LaborDemander& demander, Government& government
) noexcept {
    demander.payWage([&](const Wage wage) noexcept -> Money {
        const auto afterTax = government.payIncomeTax(static_cast<Money>(wage));
        const auto withdraw = finance.tryWithdraw(static_cast<Budget>(afterTax));
        assert(withdraw.isZeroOrMore());
        return withdraw;
    });
}

inline void workAndReceiveWage(HHoldFinance& finance, LaborSupplier& supplier) noexcept {
    supplier.work(finance.makeDepositFn());
}

inline void finalizeAccounts(FirmFinance& finance, Government& government) noexcept {
    finance.finalizeAccounts(
        [&](const Money profit) noexcept -> Money { return government.payCorporateTax(profit); },
        [&](const Money profit) noexcept -> Money {
            return government.subsideLossMakingCompany(profit);
        }
    );
}

inline void provideUnemploymentBenefit(
    HHoldFinance& finance, LaborSupplier& laborSupplier, Government& government
) noexcept {
    const auto benefit = government.provideUnemploymentBenefit(laborSupplier.wage());
    finance.deposit(benefit);
}

inline void logging(FinanceDropBox& dropBox, FirmFinance& finance) noexcept {
    dropBox.firmAssets.add(finance.asset());
}

inline void logging(FinanceDropBox& dropBox, HHoldFinance& finance) noexcept {
    dropBox.hholdAssets.add(finance.asset());
}
}  // namespace abm::end_month