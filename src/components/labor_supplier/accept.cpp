#include "components/labor_supplier.hpp"

#include "world/message.hpp"

namespace labor_supplier {
void JobHunter::accept() {
    if (not isPosting_) return;
    for (SafePtr<world::LaborEntry> myEntry : myEntries_) {
        if (not myEntry->isOffer) continue;
        myEntry->isAccept = true;
        acceptedEntry_    = myEntry;
    }
}
}  // namespace labor_supplier