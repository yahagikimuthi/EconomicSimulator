#include "strategies/labor_supplier/accept.hpp"

#include <ranges>

#include "world/message.hpp"

namespace labor_supplier {
void acceptOffer(AcceptOfferView view) {
    if (not view.isPosting()) return;
    for (const auto i : std::views::iota(0UZ, view.myEntryCnt())) {
        auto [request, myEntry] = view.myEntry(i);
        if (not myEntry.isOffer_) continue;
        myEntry.isAccept_ = true;
        view.recordAcceptance(myEntry);
        view.resign();
        return;
    }
}
}  // namespace labor_supplier