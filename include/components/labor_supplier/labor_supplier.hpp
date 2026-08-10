#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>
#include <pcg_random.hpp>
#include <vector>

#include "components/labor_supplier/job_hunter.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace labor::supplier {
class Employment {
  public:
    Employment(const double productPower) : productPower_{productPower} {}
    auto isEmployed() const -> bool { return rosterEntry_.has_value(); }
    void startWorking(world::RosterEntry& rosterEntry) {
        resign();
        rosterEntry_ = rosterEntry;
    }
    auto contractFirmId() const -> AgentID {
        return isEmployed() ? rosterEntry_->firmId() : AgentID{-1};
    }
    auto wage() const -> Wage POST(wage : wage >= Wage{0.0}) {
        return isEmployed() ? rosterEntry_->wage : Wage{0.0};
    }
    void work() {
        if (not isEmployed()) return;
        rosterEntry_->addInput(productPower_);
    }
    auto productPower() const -> double { return productPower_; }
    void updateStatus() {
        if (not isEmployed()) return;
        if (not rosterEntry_->isOccupied) rosterEntry_.reset();
    }

  private:
    void resign() {
        if (not isEmployed()) return;
        rosterEntry_->resign();
    }

    std::optional<world::RosterEntry&> rosterEntry_{std::nullopt};
    const double                       productPower_;
};

class LaborSupplier {
  public:
    LaborSupplier(
        const pcg32        rng,
        const JobHunter&&  jobHunter,
        const Employment&& employment,
        const double       jobSearchThreshold
    );
    void entry(const AgentID id, tbb::concurrent_vector<world::LaborRequest>& requestBox) {
        employment_.updateStatus();
        if (not shouldSearchJob()) return;
        if (requestBox.empty()) return;

        using Request = world::LaborRequest;
        auto isAligned{[&](const Request& req) -> bool {
            if (req.firmID == employment_.contractFirmId()) return false;
            if (req.wage < employment_.wage()) return false;
            return true;
        }};
        auto makeEntrySheet{[&](Request& req) -> world::LaborEntry& {
            return req.entry(id, employment_.productPower());
        }};
        jobHunter_.entry(isAligned, makeEntrySheet, requestBox);
    }

    void accept() { jobHunter_.accept(); }
    void recordRosterEntry() {
        const std::optional<world::LaborEntry&> acceptedEntry{jobHunter_.acceptedEntry()};
        if (not acceptedEntry) return;
        employment_.startWorking(*acceptedEntry->rosterEntry);
    }
    void endStep(world::CensusDropBox& dropBox) {
        dropBox.wages.emplace_back(wage().value());
        jobHunter_.endStep();
    }
    void product() { employment_.work(); }
    auto wage() const -> Money POST(wage : wage >= Money{0.0}) {
        return static_cast<Money>(employment_.wage());
    }

  private:
    auto shouldSearchJob() const -> bool {
        if (not employment_.isEmployed()) return true;
        return helper::rand(rng_) < jobSearchThreshold_;
    }

    mutable pcg32 rng_;
    JobHunter     jobHunter_;
    Employment    employment_;
    const double  jobSearchThreshold_;
};
}  // namespace labor::supplier