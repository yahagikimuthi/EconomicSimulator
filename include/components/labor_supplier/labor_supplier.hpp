#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>
#include <pcg_random.hpp>
#include <vector>

#include "components/labor_supplier/job_hunter.hpp"
#include "components/util.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"
#include "world/message.hpp"

namespace abm::labor::supplier {
class Employment {
  public:
    Employment(const double productPower) : productPower_{productPower} {}
    auto isEmployed() const -> bool { return rosterEntry_.has_value(); }
    void startWorking(RosterEntry& rosterEntry) {
        resign();
        rosterEntry_ = rosterEntry;
    }
    auto contractFirmId() const -> AgentID {
        return isEmployed() ? rosterEntry_->firmId() : AgentID{-1};
    }
    auto wage() const -> Wage POST(wage : wage >= Wage{0.0}) {
        return isEmployed() ? rosterEntry_->wage : Wage{0.0};
    }
    void work(const Market phase) {
        if (not isEmployed()) return;
        if (shouldWork(phase)) rosterEntry_->addInput(productPower_);
    }

    auto productPower() const -> double { return productPower_; }

    void updateStatus() {
        if (not isEmployed()) return;
        if (not rosterEntry_->isOccupied) rosterEntry_.reset();
    }

  private:
    auto shouldWork(const Market phase) const -> bool { return rosterEntry_->firmType() == phase; }
    void resign() {
        if (not isEmployed()) return;
        rosterEntry_->resign();
    }

    std::optional<RosterEntry&> rosterEntry_{std::nullopt};
    const double                productPower_;
};

class [[nodiscard]] JobSearchThreshold {
  public:
    JobSearchThreshold(const RandomGenerator rng, const double threshold)
        : rng_{rng}, threshold_{threshold} {}
    auto shouldSearch() const -> bool { return threshold_ < rng_.rand(); }

  private:
    mutable RandomGenerator rng_;
    const double            threshold_;
};
}  // namespace abm::labor::supplier

namespace abm {
class LaborSupplier {
  public:
    LaborSupplier(
        const labor::supplier::JobHunter&&        jobHunter,
        const labor::supplier::Employment&&       employment,
        const labor::supplier::JobSearchThreshold jobSearchThreshold
    )
        : jobHunter_{jobHunter}, employment_{employment}, jobSearchThreshold_{jobSearchThreshold} {}

    void entry(const AgentID id, const LaborMarket::RequestBoxT& requestBox) {
        employment_.updateStatus();
        if (not jobSearchThreshold_.shouldSearch()) return;
        if (requestBox.empty()) return;

        using Request = LaborRequest;
        auto isAligned{[&](const Request& req) -> bool {
            if (req.firmID == employment_.contractFirmId()) return false;
            if (req.wage < employment_.wage()) return false;
            return true;
        }};
        auto makeEntrySheet{[&](Request& req) -> LaborEntry& {
            return req.entry(id, employment_.productPower());
        }};
        jobHunter_.entry(isAligned, makeEntrySheet, requestBox);
    }

    void accept() { jobHunter_.accept(); }

    void recordRosterEntry() {
        const std::optional<LaborEntry&> acceptedEntry{jobHunter_.acceptedEntry()};
        if (not acceptedEntry) return;
        employment_.startWorking(*acceptedEntry->rosterEntry);
    }

    void endStep(CensusDropBox& dropBox) {
        dropBox.wages.emplace_back(wage().value());
        jobHunter_.endStep();
    }

    void product(const Market phase) { employment_.work(phase); }

    auto wage() const -> Money POST(wage : wage >= Money{0.0}) {
        return static_cast<Money>(employment_.wage());
    }

  private:
    labor::supplier::JobHunter          jobHunter_;
    labor::supplier::Employment         employment_;
    labor::supplier::JobSearchThreshold jobSearchThreshold_;
};
}  // namespace abm