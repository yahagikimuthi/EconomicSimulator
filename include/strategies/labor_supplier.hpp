#pragma once

#include <tbb/concurrent_vector.h>
#include <cstddef>

#include "components/labor_supplier.hpp"
#include "config.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"

namespace labor_supplier {
struct [[nodiscard]] JobEntryView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }
    void entry(
        const world::LaborRequest&                                request,
        const tbb::concurrent_vector<world::LaborEntry>::iterator entryIt  // NOLINT
    ) {
        comp_.posting_.myEntries_.emplace_back(&request, &*entryIt);
    }
    auto productPower() const -> double { return comp_.parameter_.productPower_; }
};
void jobEntry(
    JobEntryView                                 view,
    const int                                    id,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    const int                                    entryCnt = config::labor_supplier::jobEntryCnt
);

struct [[nodiscard]] AcceptOfferView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto myEntryCnt() const -> std::size_t { return comp_.posting_.myEntries_.size(); }
    auto myEntry(const std::size_t idx)
        -> std::pair<const world::LaborRequest&, world::LaborEntry&> {
        auto& [request, myEntry] = comp_.posting_.myEntries_[idx];
        return {*request, *myEntry};
    }
    void setContraction(const int firmId, const double wage) {
        comp_.contraction_.firmID_ = firmId, comp_.contraction_.wage_ = wage;
    }
    auto isPosting() const -> bool { return comp_.posting_.isPosting_; }
};

void acceptOffer(AcceptOfferView view);
void logging(world::CensusDropBox& dropBox, const Component& comp);
void reset(Component& comp);
}  // namespace labor_supplier