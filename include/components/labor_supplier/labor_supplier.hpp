#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>
#include <pcg_random.hpp>
#include <vector>

#include "components/labor_supplier/employment.hpp"
#include "components/labor_supplier/job_hunter.hpp"
#include "core/base.hpp"
#include "core/forward.hpp"
#include "util.hpp"
#include "world/common.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
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

namespace abm::labor::supplier {
class LaborSupplier final {
  public:
    [[nodiscard]] LaborSupplier(
        const JobHunter&&  jobHunter,
        const Employment&& employment,
        const JobSearch    jobSearchThreshold
    )
        : jobHunter_{jobHunter}, employment_{employment}, jobSearch_{jobSearchThreshold} {}

    void entry(const AgentID id, LaborMarket& market) {
        employment_.updateStatus();
        if (not employment_.isEmployed()) return;
        if (not jobSearch_()) return;
        auto isAligned = [&] [[nodiscard]] (const Request& req) -> bool {
            if (req.firmID == employment_.contractFirmId()) return false;
            if (req.wage < employment_.wage()) return false;
            return true;
        };
        auto makeEntrySheet = [&](Request& req) -> LaborEntry& {
            return req.entry(id, employment_.productPower());
        };
        jobHunter_.entry(isAligned, makeEntrySheet, market);
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
    JobHunter  jobHunter_;
    Employment employment_;
    JobSearch  jobSearch_;
};
}  // namespace abm::labor::supplier

namespace abm {
using LaborSupplier = labor::supplier::LaborSupplier;
}