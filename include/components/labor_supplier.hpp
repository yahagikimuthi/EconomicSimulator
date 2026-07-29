#pragma once

#include <tbb/concurrent_vector.h>
#include <concepts>
#include <config.hpp>
#include <helper.hpp>
#include <pcg_random.hpp>
#include <vector>

#include "core/base.hpp"
#include "core/forward.hpp"
#include "world/message.hpp"

namespace labor_supplier {

template <typename T>
concept HasIsEligibleRequest = requires(T t, const world::LaborRequest& request) {
    { t.isEligibleRequest(request) } -> std::same_as<bool>;
};

class JobHunter {
  public:
    JobHunter(pcg32& masterRng);
    void entry(
        const int                                    id,
        const HasIsEligibleRequest auto&             hasIsEligibleRequest,
        const double                                 productPower,
        tbb::concurrent_vector<world::LaborRequest>& requestBox,
        const int                                    entryCnt = config::labor_supplier::jobEntryCnt
    );
    void accept();
    void endStep() { myEntries_.clear(), acceptedEntry_ = nullptr, isPosting_ = false; }
    auto acceptedEntry() const -> SafePtr<world::LaborEntry> { return acceptedEntry_; }

  private:
    mutable pcg32                           rng_;
    std::vector<SafePtr<world::LaborEntry>> myEntries_;
    SafePtr<world::LaborEntry>              acceptedEntry_{nullptr};
    bool                                    isPosting_{false};
};

class Employment {
  public:
    auto isEmployed() const -> bool { return rosterEntry_.hasValue(); }
    void setRosterEntry(const SafePtr<world::RosterEntry> rosterEntry) {
        resign();
        rosterEntry_ = rosterEntry;
    }
    auto contractFirmId() const -> int {
        return isEmployed() ? rosterEntry_->companyBoard.firmId : -1;
    }
    auto wage() const -> double { return isEmployed() ? rosterEntry_->wage : 0.0; }
    void product(const double productPower) {
        if (not isEmployed()) return;
        auto& workspace = rosterEntry_->workspace;
        workspace.totalLaborInput += workspace.firmProductPower * productPower;
    }
    void updateStatus() {
        if (not isEmployed()) return;
        if (not rosterEntry_->isOccupied) rosterEntry_ = nullptr;
    }

  private:
    void resign() {
        if (not isEmployed()) return;
        rosterEntry_->companyBoard.resignationBox.emplace_back(rosterEntry_);
    }

    SafePtr<world::RosterEntry> rosterEntry_{nullptr};
};

class LaborSupplier {
  public:
    LaborSupplier(pcg32& masterRng);
    void entry(const int id, tbb::concurrent_vector<world::LaborRequest>& requestBox);
    void accept() { jobHunter_.accept(); }
    void recordRosterEntry() {
        const SafePtr<world::LaborEntry> acceptedEntry{jobHunter_.acceptedEntry()};
        if (not acceptedEntry) return;
        employment_.setRosterEntry(acceptedEntry->rosterEntry);
    }
    void endStep(world::CensusDropBox& dropBox) {
        dropBox.wages.emplace_back(wage());
        jobHunter_.endStep();
    }
    void product() { employment_.product(productPower_); }
    auto wage() const -> double { return employment_.wage(); }
    auto isEligibleRequest(const world::LaborRequest& request) const -> bool {
        if (request.firmID == employment_.contractFirmId()) return false;
        if (request.wage < employment_.wage()) return false;
        return true;
    }

  private:
    void updateRosterEntry() { employment_.updateStatus(); }
    auto shouldSearchJob() const -> bool;

    mutable pcg32 rng_;
    JobHunter     jobHunter_;
    Employment    employment_{};
    const double  productPower_;
    const double  jobSearchThreshold_;
};
}  // namespace labor_supplier