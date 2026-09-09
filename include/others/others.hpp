#pragma once

#include "components/finance/firm_finance.hpp"
#include "components/finance/others_finance.hpp"
#include "components/government.hpp"
#include "components/labor_supplier/labor_supplier.hpp"

namespace abm {
inline void finalizeAccounts(FirmFinance& finance, Government government) noexcept {
    finance.finalizeAccounts([&](const Money profit) noexcept -> Money {
        return government.payCorporateTax(profit);
    });
}

inline void provideUnemploymentBenefit(
    HHoldFinance& finance, LaborSupplier& laborSupplier, Government& government
) noexcept;
}  // namespace abm