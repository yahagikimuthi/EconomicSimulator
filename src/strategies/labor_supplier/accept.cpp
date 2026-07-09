#include "strategies/labor_supplier.hpp"

#include <cstddef>

#include "world/message.hpp"

namespace labor_supplier {
void acceptOffer(AcceptOfferView view) {
    for (std::size_t i{}; i < view.myEntryCnt(); ++i) {
        auto [request, myEntry] = view.getMyEntry(i);
        if (not myEntry.isAccept_) continue;
        view.setContraction(request.firmID_, request.wage_);
        return;
    }
}
}  // namespace labor_supplier