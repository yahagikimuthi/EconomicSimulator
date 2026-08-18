#pragma once

#include "world/goods.hpp"
#include "world/labor.hpp"

namespace abm {
inline void RosterEntry::addInput(const double productPower) { workspace.addInput(productPower); }
}  // namespace abm