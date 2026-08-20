#pragma once

#include "components/common.hpp"
#include "world/common.hpp"

namespace abm::firm_finance {
void logging(CensusDropBox& dropBox, const FirmFinance& finance) noexcept {
    finance.endStep(dropBox);
}
}  // namespace abm::firm_finance

namespace abm::hhold_finance {
void logging(CensusDropBox& dropBox, const HHoldFinance& finance) noexcept {
    finance.endStep(dropBox);
}
}  // namespace abm::hhold_finance