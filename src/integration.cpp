#include "world/consumer_goods.hpp"
#include "world/labor.hpp"

namespace abm::labor {
void RosterEntry::addInput(const double productPower) noexcept {
    workspace_.addInput(productPower);
}
}  // namespace abm::labor