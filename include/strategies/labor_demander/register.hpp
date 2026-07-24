#pragma once

#include <tbb/concurrent_vector.h>
#include <cstddef>

#include "components/labor_demander.hpp"
#include "core/base.hpp"
#include "world/message.hpp"

namespace labor_demander {
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
    auto addRoster(
        const double wage, world::CompanyBoard& companyBoard, world::Workspace& workspace
    ) -> SafePtr<world::RosterEntry> {
        auto& hr = comp_.humanResources_;
        if (hr.emptyRosterPool_.empty()) {
            return &hr.companyBoard_.roster_.emplace_back(wage, companyBoard, workspace);
        }
        SafePtr<world::RosterEntry> newEntry = hr.emptyRosterPool_.back();
        newEntry->wage_                      = wage;
        newEntry->isOccupied_                = true;
        newEntry->isLaidOff                  = true;
        hr.emptyRosterPool_.pop_back();
        return newEntry;
    }

    void applicantNumPlus(const std::size_t n) {
        comp_.employmentLedger.applicantNum_ += static_cast<int>(n);
    }
    auto myCompanyBoard() -> world::CompanyBoard& { return comp_.humanResources_.companyBoard_; }
};

void registerMember(RegisterMemberView view, world::Workspace& workspace);

struct [[nodiscard]] AcceptResignationView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    void addEmptyRosterPool(const SafePtr<world::RosterEntry> emptyRoster) {
        comp_.humanResources_.emptyRosterPool_.emplace_back(emptyRoster);
    }
    auto resignationBox() -> tbb::concurrent_vector<SafePtr<world::RosterEntry>>& {
        return comp_.humanResources_.companyBoard_.resignationBox_;
    }
    void wageMinus(const double minus) { comp_.humanResources_.sumWage_ -= minus; }
};

void acceptResignation(AcceptResignationView view);
}  // namespace labor_demander