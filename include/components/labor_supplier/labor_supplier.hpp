#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>
#include <pcg_random.hpp>
#include <vector>

#include "components/labor_supplier/employment.hpp"
#include "components/labor_supplier/job_hunter.hpp"
#include "core/assertion.hpp"
#include "core/forward.hpp"
#include "setting.hpp"
#include "util.hpp"
#include "world/common.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
class LikelihoodChangingJob final {
  public:
    [[nodiscard]] explicit constexpr LikelihoodChangingJob(RandomGenerator& masterRng) noexcept
        : rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}},
          threshold_{masterRng.random(setting::changingJobThreshold)} {}

    auto shouldChangingJobs() const noexcept -> bool { return threshold_ < rng_.rand(); }

  private:
    mutable RandomGenerator rng_;
    const double            threshold_;
};
}  // namespace abm::labor::supplier

namespace abm::labor::supplier {
class LaborSupplier final {
  public:
    [[nodiscard]] explicit constexpr LaborSupplier(RandomGenerator& masterRng) noexcept
        : employment_{masterRng}, likelihoodChangingJob_{masterRng} {}

    void entry(const AgentID id, LaborMarket& market) noexcept {
        employment_.updateStatus();
        if (not shouldSearch()) return;
        jobHunter_.entry(
            [&] [[nodiscard]] (const Request& req) -> bool { return isAligned(req); },
            [&] [[nodiscard]] (Request & req) -> Entry& { return makeEntrySheet(id, req); },
            market
        );
    }

    void accept() noexcept { jobHunter_.accept(); }

    void recordRosterEntry() noexcept {
        const auto acceptedEntry = jobHunter_.huntedResult();
        if (not acceptedEntry) return;
        employment_.startWorking(*acceptedEntry->rosterEntry);
    }

    void endStep(CensusDropBox& dropBox) noexcept {
        dropBox.wages.emplace_back(wage().value());
        jobHunter_.endStep();
    }

    void product(const Market phase) noexcept { employment_.work(phase); }

    [[nodiscard]] auto wage() const noexcept -> Money {
        const auto out = employment_.wage();
        ASSERT(out >= Wage{0.0});
        return static_cast<Money>(out);
    }

  private:
    [[nodiscard]] auto shouldSearch() const noexcept -> bool {
        if (not employment_.isEmployed()) return true;
        if (likelihoodChangingJob_.shouldChangingJobs()) return true;
        return false;
    }

    [[nodiscard]] auto isAligned(const Request& request) const noexcept -> bool {
        if (request.firmID == employment_.contractFirmId()) return false;
        if (request.wage < employment_.wage()) return false;
        return true;
    }

    [[nodiscard]] auto makeEntrySheet(const AgentID id, Request& request) const noexcept -> Entry& {
        return request.entry(id, employment_.productPower());
    }

    JobHunter             jobHunter_;
    Employment            employment_;
    LikelihoodChangingJob likelihoodChangingJob_;
};
}  // namespace abm::labor::supplier

namespace abm {
using LaborSupplier = labor::supplier::LaborSupplier;
}