#include "world/goods.hpp"
#include "world/labor.hpp"

namespace abm {
void RosterEntry::addInput(const double productPower) noexcept {
    workspace_.addInput(productPower);
}
}  // namespace abm