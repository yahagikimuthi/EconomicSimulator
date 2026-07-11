#include "strategies/labor_supplier.hpp"

#include <ranges>

#include "world/message.hpp"

namespace labor_supplier {
void acceptOffer(AcceptOfferView view) {
    for (const auto i : std::views::iota(0UZ, view.myEntryCnt())) {
        auto [request, myEntry] = view.getMyEntry(i);
        if (not myEntry.isOffer_) continue;
        view.setContraction(request.firmID_, request.wage_);
        return;
    }
}
}  // namespace labor_supplier