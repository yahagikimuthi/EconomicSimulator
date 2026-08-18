#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>
#include <pcg_random.hpp>
#include <span>
#include <vector>

#include "components/labor_supplier/job_hunter.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"
#include "util.hpp"
#include "world/common.hpp"
#include "world/integration.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
class Employment final {
  public:
    [[nodiscard]] Employment(const double productPower) : productPower_{productPower} {}

    [[nodiscard]] auto isEmployed() const -> bool { return rosterEntry_.has_value(); }

    void startWorking(RosterEntry& rosterEntry) {
        resign();
        rosterEntry_ = rosterEntry;
    }

    [[nodiscard]] auto contractFirmId() const -> AgentID {
        return isEmployed() ? rosterEntry_->firmId() : AgentID{-1};
    }

    [[nodiscard]] auto wage() const -> Wage POST(wage : wage >= Wage{0.0}) {
        return isEmployed() ? rosterEntry_->wage : Wage{0.0};
    }

    void work(const Market phase) {
        if (shouldWork(phase)) rosterEntry_->addInput(productPower_);
    }

    [[nodiscard]] auto productPower() const -> double { return productPower_; }

    void updateStatus() {
        if (not isEmployed()) return;
        if (not rosterEntry_->isOccupied) rosterEntry_.reset();
    }

  private:
    [[nodiscard]] auto shouldWork(const Market phase) const -> bool {
        if (not isEmployed()) return false;
        return rosterEntry_->firmType() == phase;
    }

    void resign() {
        if (not isEmployed()) return;
        rosterEntry_->resign();
    }

    std::optional<RosterEntry&> rosterEntry_{std::nullopt};
    const double                productPower_;
};

class JobSearch {
  public:
    [[nodiscard]] JobSearch(const RandomGenerator rng, const double threshold)
        : rng_{rng}, threshold_{threshold} {}
    auto operator()() const -> bool { return threshold_ < rng_.rand(); }

  private:
    mutable RandomGenerator rng_;
    const double            threshold_;
};
}  // namespace abm::labor::supplier

namespace abm {
class LaborSupplier final {
  public:
    [[nodiscard]] LaborSupplier(
        const labor::supplier::JobHunter&&  jobHunter,
        const labor::supplier::Employment&& employment,
        const labor::supplier::JobSearch    jobSearchThreshold
    )
        : jobHunter_{jobHunter}, employment_{employment}, jobSearch_{jobSearchThreshold} {}

    void entry(const AgentID id, const std::span<RefWrap<LaborRequest>> requestBox) {
        employment_.updateStatus();
        if (not employment_.isEmployed()) return;
        if (not jobSearch_()) return;
        if (requestBox.empty()) return;

        using Request  = LaborRequest;
        auto isAligned = [&] [[nodiscard]] (const Request& req) -> bool {
            if (req.firmID == employment_.contractFirmId()) return false;
            if (req.wage < employment_.wage()) return false;
            return true;
        };
        auto makeEntrySheet = [&](Request& req) -> LaborEntry& {
            return req.entry(id, employment_.productPower());
        };
        jobHunter_.entry(isAligned, makeEntrySheet, requestBox);
    }

    void accept() { jobHunter_.accept(); }

    void recordRosterEntry() {
        const auto acceptedEntry = jobHunter_.huntedResult();
        if (not acceptedEntry) return;
        employment_.startWorking(*acceptedEntry->rosterEntry);
    }

    void endStep(CensusDropBox& dropBox) {
        dropBox.wages.emplace_back(wage().value());
        jobHunter_.endStep();
    }

    void product(const Market phase) { employment_.work(phase); }

    [[nodiscard]] auto wage() const -> Money POST(wage : wage >= Money{0.0}) {
        return static_cast<Money>(employment_.wage());
    }

  private:
    labor::supplier::JobHunter  jobHunter_;
    labor::supplier::Employment employment_;
    labor::supplier::JobSearch  jobSearch_;
};
}  // namespace abm