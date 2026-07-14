#pragma once

#include <tbb/concurrent_vector.h>
#include <functional>

#include "components/labor_demander.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"

namespace labor_demander {
struct [[nodiscard]] PostJobView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    void plan(const double wage, const int employ) {
        comp_.plan_.wage_ = wage, comp_.plan_.employ_ = employ;
    }
    void myRequest(const tbb::concurrent_vector<world::LaborRequest>::iterator it) {  // NOLINT
        comp_.posting_.myRequest_ = &*it;
    }
    void posting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }
};

void postJob(
    const int                                    id,
    const bool                                   isSold,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    PostJobView                                  view
);

struct [[nodiscard]] OfferApplicantsView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto employPlan() const -> int { return comp_.plan_.employ_; }
    auto myRequest() -> world::LaborRequest& { return *comp_.posting_.myRequest_; }
    auto isPosting() const -> bool { return comp_.posting_.isPosting_; }
    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }
    void recordOffer(world::LaborEntry& entry) {
        comp_.posting_.offerApplicants_.emplace_back(entry);
    }
};

void offerApplicants(OfferApplicantsView view);

struct [[nodiscard]] RegisterMemberView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto myRequest() const -> world::LaborRequest& { return *comp_.posting_.myRequest_; }
    auto offerApplicants() const -> std::vector<std::reference_wrapper<world::LaborEntry>>& {
        return comp_.posting_.offerApplicants_;
    }
    auto isPosting() const -> bool { return comp_.posting_.isPosting_; }
    auto targetEmploy() const -> int { return comp_.plan_.employ_; }
    void updateLedger(const double wage, const int actualEmploy) {
        auto& ledger = comp_.employmentLedger;
        ledger.employing_ += actualEmploy, ledger.sumWage_ += wage * actualEmploy;
    }
};

[[nodiscard]] auto registerMember(RegisterMemberView view) -> double;

void logging(world::CensusDropBox& dropBox, const Component& comp);
void reset(Component& comp);
}  // namespace labor_demander