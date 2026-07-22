#pragma once

#include <tbb/concurrent_vector.h>

#include "components/labor_demander.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"
#include "world/message.hpp"

namespace labor_demander {
struct [[nodiscard]] PostJobView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    void plan(const double wage, const int employ, const int offer) {
        auto& plan = comp_.plan_;
        plan.wage_ = wage, plan.employ_ = employ, plan.offer_ = offer;
    }
    void myRequest(tbb::concurrent_vector<world::LaborRequest>::iterator it) {  // NOLINT
        comp_.posting_.myRequest_ = &*it;
    }
    void posting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }

    void isRecruiting(const bool isRecruiting) { comp_.plan_.isRecruiting = isRecruiting; }
};

void postJob(
    const int                                    id,
    const int                                    desiredEmploy,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    PostJobView                                  view
);

struct LayOffsView final : BaseView<Component> {
    using BaseView<Component>::BaseView;

    auto rosterSize() const -> int {
        const std::size_t rosterSize{comp_.humanResources_.companyBoard_->roster_.size()};
        return static_cast<int>(rosterSize);
    }
    auto getRosterEntry(const int idx) -> world::RosterEntry& {
        return comp_.humanResources_.companyBoard_->roster_[static_cast<std::size_t>(idx)];
    }
    void addEmptyRoster(world::RosterEntry& emptyRoster) {
        comp_.humanResources_.emptyRosterPool_.emplace_back(&emptyRoster);
    }
    auto emptyRosterPool() -> std::vector<SafePtr<world::RosterEntry>>& {
        return comp_.humanResources_.emptyRosterPool_;
    }
};

void layoffs(LayOffsView view, const int layOffsCnt);

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

struct [[nodiscard]] RegisterMemberView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto myRequest() -> world::LaborRequest& { return *comp_.posting_.myRequest_; }
    auto offerNum() const -> std::size_t { return comp_.posting_.offerApplicants_.size(); }
    auto offerApplicant(const std::size_t idx) -> world::LaborEntry& {
        return *comp_.posting_.offerApplicants_[idx];
    }
    auto isPosting() const -> bool { return comp_.posting_.isPosting_; }
    void updateLedger(const double wage, const int actualEmploy) {
        auto& ledger = comp_.employmentLedger;
        ledger.employing_ += actualEmploy, ledger.sumWage_ += wage * actualEmploy;
    }
    void applicantNumPlus(const std::size_t n) {
        comp_.employmentLedger.applicantNum_ += static_cast<int>(n);
    }
    auto myCompanyBoard() -> world::CompanyBoard& { return *comp_.humanResources_.companyBoard_; }
};

void registerMember(RegisterMemberView view);

void logging(world::CensusDropBox& dropBox, const Component& comp);
void reset(Component& comp);
}  // namespace labor_demander