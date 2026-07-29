#pragma once

#include "core/forward.hpp"

namespace firm_finance {
void logging(world::CensusDropBox& dropBox, const Component& comp);
}

namespace hhold_finance {
void logging(world::CensusDropBox& dropBox, const Component& comp);
}

namespace labor_supplier {
void logging(world::CensusDropBox& dropBox, const Component& comp);
void reset(Component& comp);
}  // namespace labor_supplier