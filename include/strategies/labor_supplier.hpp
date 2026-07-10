#pragma once

#include <tbb/concurrent_vector.h>
#include <config/init_setup.hpp>
#include <cstddef>

#include "components/labor_supplier.hpp"
#include "config/contract.hpp"
#include "core/forward.hpp"

namespace labor_supplier {
struct JobEntryView {
    explicit JobEntryView(Component& comp) : comp_{comp} {}

    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }

    void entry(
        const world::LaborRequest&                                request,
        const tbb::concurrent_vector<world::LaborEntry>::iterator entryIt  // NOLINT
    ) {
        comp_.posting_.myEntries_.emplace_back(&request, &*entryIt);
    }

    [[nodiscard]] auto productPower() const -> double { return comp_.parameter_.productPower_; }

  private:
    Component& comp_;
};
void jobEntry(
    JobEntryView                                 view,
    const int                                    id,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    const int                                    entryCnt = config::labor_supplier::jobEntryCnt
);

struct AcceptOfferView {
    explicit AcceptOfferView(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto myEntryCnt() const -> std::size_t {
        return comp_.posting_.myEntries_.size();
    }

    [[nodiscard]] auto getMyEntry(const std::size_t idx)
        -> std::pair<const world::LaborRequest&, const world::LaborEntry&> {
        auto& [request, myEntry]{ACCESS(comp_.posting_.myEntries_, idx)};
        return {*request, *myEntry};
    }

    [[nodiscard]] auto setContraction(const int firmId, const double wage) {
        comp_.contraction_.firmID_ = firmId, comp_.contraction_.wage_ = wage;
    }

  private:
    Component& comp_;
};

void acceptOffer(AcceptOfferView view);
void logging(world::CensusDropBox& dropBox, const Component& comp);
void reset(Component& comp);
}  // namespace labor_supplier