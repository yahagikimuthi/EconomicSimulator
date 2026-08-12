#pragma once

#include "components/common.hpp"
#include "world/message.hpp"

namespace abm::firm_finance {
void logging(CensusDropBox& dropBox, const FirmFinance& comp) { comp.endStep(dropBox); }
}  // namespace abm::firm_finance

namespace abm::hhold_finance {
void logging(CensusDropBox& dropBox, const HHoldFinance& comp) { comp.endStep(dropBox); }
}  // namespace abm::hhold_finance