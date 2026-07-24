#pragma once

#include <cstddef>
#include <utility>

#include "components/labor_supplier.hpp"
#include "core/base.hpp"
#include "world/message.hpp"

namespace labor_supplier {
struct [[nodiscard]] AcceptOfferView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto myEntryCnt() const -> std::size_t { return comp_.posting_.myEntries_.size(); }
    auto myEntry(const std::size_t idx)
        -> std::pair<const world::LaborRequest&, world::LaborEntry&> {
        auto& myEntry = *comp_.posting_.myEntries_[idx];
        return {*myEntry.request_, myEntry};
    }
    void recordAcceptance(const world::LaborEntry& acceptEntry) {
        comp_.posting_.acceptEntry_ = &acceptEntry;
    }
    auto isPosting() const -> bool { return comp_.posting_.isPosting_; }
    void resign() {
        if (not comp_.rosterEntry_) return;
        comp_.rosterEntry_->companyBoard_->resignationBox_.emplace_back(comp_.rosterEntry_);
    }
};

void acceptOffer(AcceptOfferView view);
}  // namespace labor_supplier