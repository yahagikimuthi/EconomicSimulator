#pragma once

#include <tbb/concurrent_vector.h>
#include <config.hpp>
#include <cstdint>
#include <helper.hpp>
#include <pcg_random.hpp>
#include <vector>

#include "core/base.hpp"
#include "core/forward.hpp"
#include "world/message.hpp"

namespace labor_supplier {
class JobHunter {
  public:
    JobHunter(pcg32& masterRng);
    void entry(
        const int                                    id,
        const int                                    contractFirmId,
        const double                                 contractWage,
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

class LaborSupplier {
  public:
    LaborSupplier(pcg32& masterRng);
    void entry(const int id, tbb::concurrent_vector<world::LaborRequest>& requestBox);
    auto isEmployed() const -> bool { return rosterEntry_.hasValue(); }
    void accept() { jobHunter_.accept(); }
    void recordRosterEntry() {
        const SafePtr<world::LaborEntry> acceptedEntry{jobHunter_.acceptedEntry()};
        if (not acceptedEntry) return;
        rosterEntry_ = acceptedEntry->rosterEntry;
    }
    void endStep(world::CensusDropBox& dropBox) {
        dropBox.wages.emplace_back(isEmployed() ? rosterEntry_->wage : 0.0);
        jobHunter_.endStep();
    }
    void product() {
        if (not isEmployed()) return;
        auto& workspace = rosterEntry_->workspace;
        workspace.totalLaborInput += productPower_ * workspace.firmProductPower;
    }
    auto wage() const -> double { return isEmployed() ? rosterEntry_->wage : 0.0; }

  private:
    void updateRosterEntry() {
        if (rosterEntry_) return;
        if (rosterEntry_->isOccupied) return;
        rosterEntry_ = nullptr;
    }
    auto shouldSearchJob() const -> bool;
    void resign() {
        if (not isEmployed()) return;
        rosterEntry_->companyBoard.resignationBox.emplace_back(rosterEntry_);
    }

    mutable pcg32               rng_;
    JobHunter                   jobHunter_;
    SafePtr<world::RosterEntry> rosterEntry_{nullptr};
    const double                productPower_;
    const double                jobSearchThreshold_;
};
}  // namespace labor_supplier