#pragma once

#include "components/common.hpp"
#include "world/message.hpp"

namespace firm_finance {
void logging(world::CensusDropBox& dropBox, const Component& comp) { comp.endStep(dropBox); }
}  // namespace firm_finance

namespace hhold_finance {
void logging(world::CensusDropBox& dropBox, const Component& comp) { comp.endStep(dropBox); }
}  // namespace hhold_finance