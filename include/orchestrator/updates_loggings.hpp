#pragma once

#include "components/common.hpp"
#include "world/common.hpp"

namespace abm::firm_finance {
void logging(CensusDropBox& dropBox, const FirmFinance& comp) noexcept { comp.endStep(dropBox); }
}  // namespace abm::firm_finance

namespace abm::hhold_finance {
void logging(CensusDropBox& dropBox, const HHoldFinance& comp) noexcept { comp.endStep(dropBox); }
}  // namespace abm::hhold_finance