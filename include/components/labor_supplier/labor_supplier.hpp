#pragma once

#include <optional>
#include <utility>

#include "components/common.hpp"
#include "components/labor_supplier/employment.hpp"
#include "components/labor_supplier/job_hunter.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/date.hpp"
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
        if (not shouldSearch()) return;
        jobHunter_.entry(
            id, employment_.makeIsAlignedRequestFn(), employment_.makeEntrySheetFn(id), market
        );
    }

    void accept() noexcept { jobHunter_.accept(); }

    template <DepositFn F>
    void recordRosterEntry(F&& depositFn) noexcept {
        const auto acceptedEntry = jobHunter_.huntedResult();
        if (acceptedEntry)
            employment_.startWorking(
                acceptedEntry->takeoutRosterEntry(), std::forward<F>(depositFn)
            );
        jobHunter_.reset();
    }

    template <DepositFn F>
    void work(F&& depositFn, const Date& today) noexcept {
        employment_.work(std::forward<F>(depositFn), today);
    }

    [[nodiscard]] auto wage() const noexcept -> Budget {
        const auto out = employment_.wage();
        ASSERT(out.isZeroOrMore());
        return static_cast<Budget>(out);
    }

  private:
    [[nodiscard]] auto shouldSearch() const noexcept -> bool {
        if (not employment_.isEmployed()) return true;
        if (likelihoodChangingJob_.shouldChangingJobs()) return true;
        return false;
    }

    void reset(CensusDropBox& dropBox) noexcept {
        dropBox.wages.emplace_back(wage().value());
        jobHunter_.reset();
    }

    JobHunter<>           jobHunter_;
    Employment            employment_;
    LikelihoodChangingJob likelihoodChangingJob_;
};
}  // namespace abm::labor::supplier

namespace abm {
using LaborSupplier = labor::supplier::LaborSupplier;
}