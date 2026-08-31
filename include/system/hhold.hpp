#pragma once

#include "components/labor_supplier/labor_supplier.hpp"

namespace abm::hhold {
inline void work(LaborSupplier& laborSupplier) noexcept { laborSupplier.product(); }
}  // namespace abm::hhold