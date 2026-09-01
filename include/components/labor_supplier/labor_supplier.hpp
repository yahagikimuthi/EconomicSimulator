#pragma once

#include <optional>
#include <utility>

#include "components/common.hpp"
#include "components/labor_supplier/employment.hpp"
#include "components/labor_supplier/job_hunter.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "world/drop_box.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
class LikelihoodChangingJob final {
  public:
    explicit LikelihoodChangingJob(RandomGenerator& masterRng) noexcept
        : rng_{{masterRng.makeUint64(), masterRng.makeUint64()}},
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
    explicit LaborSupplier(RandomGenerator& masterRng) noexcept
        : jobHunter_{masterRng}, employment_{masterRng}, likelihoodChangingJob_{masterRng} {}

    void entry(const AgentID id, Market& market) noexcept {
        employment_.updateStatus();
        if (not shouldSearch()) return;
        jobHunter_.entry(
            id,
            [&] [[nodiscard]] (const Request& req) noexcept -> bool { return isAligned(req); },
            [&] [[nodiscard]] (Request & req) noexcept -> Entry& {
                return makeEntrySheet(id, req);
            },
            market
        );
    }

    void accept() noexcept { jobHunter_.accept(); }

    void recordRosterEntry() noexcept {
        const auto acceptedEntry = jobHunter_.huntedResult();
        if (not acceptedEntry) return;
        employment_.startWorking(acceptedEntry->rosterEntry());
    }

    template <AssetPlusFn F>
    void endStep(F&& assetPlus, CensusDropBox& dropBox) noexcept {
        std::forward<F>(assetPlus)(wage());
        reset(dropBox);
    }

    void product() noexcept { employment_.work(); }

    [[nodiscard]] auto wage() const noexcept -> Money {
        const auto out = employment_.wage();
        ASSERT(out.isZeroOrMore());
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

    void reset(CensusDropBox& dropBox) noexcept {
        dropBox.wages.emplace_back(wage().value());
        jobHunter_.reset();
    }

    JobHunter             jobHunter_;
    Employment            employment_;
    LikelihoodChangingJob likelihoodChangingJob_;
};
}  // namespace abm::labor::supplier

namespace abm {
using LaborSupplier = labor::supplier::LaborSupplier;
}