#pragma once

#include "components/common.hpp"
#include "components/government.hpp"
#include "world/common.hpp"

namespace abm::firm_finance {
void endAllStep(FirmFinance& finance, Government& government, CensusDropBox& dropBox) noexcept {
    finance.endStep(
        [&](const Money profit) -> Money { return government.collectCorporateTax(profit); }, dropBox
    );
}
}  // namespace abm::firm_finance

namespace abm::hhold_finance {
void endAllStep(const HHoldFinance& finance, CensusDropBox& dropBox) noexcept {
    finance.endStep(dropBox);
}
}  // namespace abm::hhold_finance