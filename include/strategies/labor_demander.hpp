#pragma once

#include <tbb/concurrent_vector.h>
#include <functional>

#include "components/labor_demander.hpp"
#include "core/forward.hpp"

namespace labor_demander {
struct PostJobView {
    explicit PostJobView(Component& comp) : comp_{comp} {}

    void plan(const double wage, const int employ) {
        comp_.plan_.wage_ = wage, comp_.plan_.employ_ = employ;
    }
    void myRequest(const tbb::concurrent_vector<world::LaborRequest>::iterator it) {  // NOLINT
        comp_.posting_.myRequest_ = &(*it);
    }
    void posting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }

  private:
    Component& comp_;
    friend struct CalcNextWageView;
    friend struct CalcNextEmployView;
};

void postJob(
    const int                                    id,
    const double                                 asset,
    const bool                                   isSold,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    PostJobView                                  view
);

struct OfferApplicantsView {
    explicit OfferApplicantsView(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto employPlan() const -> int { return comp_.plan_.employ_; }

    [[nodiscard]] auto myRequest() const -> world::LaborRequest& {
        return *comp_.posting_.myRequest_;
    }
    [[nodiscard]] auto isPosting() const -> bool { return comp_.posting_.isPosting_; }

    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }

    void recordOffer(world::LaborEntry& entry) {
        comp_.posting_.offerApplicants_.emplace_back(entry);
    }

  private:
    Component& comp_;
};

void offerApplicants(OfferApplicantsView view);

struct RegisterMemberView {
    explicit RegisterMemberView(Component& comp) : comp_{comp} {};

    [[nodiscard]] auto myRequest() const -> world::LaborRequest& {
        return *comp_.posting_.myRequest_;
    }
    [[nodiscard]] auto offerApplicants() const
        -> std::vector<std::reference_wrapper<world::LaborEntry>>& {
        return comp_.posting_.offerApplicants_;
    }
    [[nodiscard]] auto isPosting() const -> bool { return comp_.posting_.isPosting_; }

    [[nodiscard]] auto targetEmploy() const -> int { return comp_.plan_.employ_; }

    void updateLedger(const double wage, const int actualEmploy) {
        auto& ledger = comp_.employmentLedger;
        ledger.employing_ += actualEmploy, ledger.sumWage_ += wage * actualEmploy;
    }

  private:
    Component& comp_;
};

[[nodiscard]] auto registerMember(RegisterMemberView view) -> double;

void logging(world::CensusDropBox& dropBox, const Component& comp);
void reset(Component& comp);
}  // namespace labor_demander