#pragma once

#include <tbb/concurrent_vector.h>
#include <functional>
#include <vector>

#include "components/labor_demander.hpp"
#include "core/base.hpp"
#include "world/message.hpp"

namespace labor_demander::internal {
void sortApplicants(
    const int                                               offer,
    std::vector<std::reference_wrapper<world::LaborEntry>>& applicants,
    tbb::concurrent_vector<world::LaborEntry>&              entryBox
);
}

namespace labor_demander {
struct [[nodiscard]] OfferApplicantsView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto offerPlan() const -> int { return comp_.plan_.offer_; }
    auto myRequest() -> world::LaborRequest& { return *comp_.posting_.myRequest_; }
    auto isPosting() const -> bool { return comp_.posting_.isPosting_; }
    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }
    void recordOffer(world::LaborEntry& entry) {
        comp_.posting_.offerApplicants_.emplace_back(&entry);
    }
};

void offerApplicants(OfferApplicantsView view);
}  // namespace labor_demander