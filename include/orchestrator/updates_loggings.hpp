#pragma once

#include "components/common.hpp"
#include "components/government.hpp"
#include "components/labor_supplier/labor_supplier.hpp"
#include "world/common.hpp"

namespace abm::firm_finance {
void endAllStep(FirmFinance& finance, Government& government, CensusDropBox& dropBox) noexcept {
    finance.endStep(
        [&, count = 0](const Money profit) mutable -> Money {
            ASSERT(count == 0);
            ++count;
            return government.collectCorporateTax(profit);
        },
        dropBox
    );
}
}  // namespace abm::firm_finance

namespace abm::hhold_finance {
void endAllStep(
    HHoldFinance&        finance,
    const LaborSupplier& laborSupplier,
    Government&          government,
    CensusDropBox&       dropBox
) noexcept {
    const auto wage                = laborSupplier.wage();
    const auto unemploymentBenefit = government.provideUnemploymentBenefit(static_cast<Wage>(wage));
    finance.assetPlus(unemploymentBenefit);
    finance.endStep(dropBox);
}
}  // namespace abm::hhold_finance