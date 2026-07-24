#pragma once

#include <tbb/concurrent_vector.h>
#include <pcg_random.hpp>

#include "components/labor_supplier.hpp"
#include "config.hpp"
#include "core/base.hpp"
#include "world/message.hpp"

namespace labor_supplier::internal {
void pickSample(
    tbb::concurrent_vector<world::LaborRequest>&              requestBox,
    std::vector<std::reference_wrapper<world::LaborRequest>>& sampleRequests,
    pcg32&                                                    rng,
    const int sampleCnt = config::labor_supplier::jobSampleCnt
);

void sortSample(
    std::vector<std::reference_wrapper<world::LaborRequest>>& sortRequests,
    const int entryCnt = config::labor_supplier::jobEntryCnt
);
}  // namespace labor_supplier::internal

namespace labor_supplier {
struct [[nodiscard]] UpdateRosterEntryView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto rosterEntry() -> SafePtr<world::RosterEntry> { return comp_.rosterEntry_; }
};

void updateRosterEntry(UpdateRosterEntryView view);

struct [[nodiscard]] JobEntryView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }
    auto contractFirmId() const -> int {
        if (not comp_.rosterEntry_) return -1;
        return comp_.rosterEntry_->companyBoard_->firmId_;
    }
    auto contractWage() const -> double {
        if (not comp_.rosterEntry_) return 0.0;
        return comp_.rosterEntry_->wage_;
    }
    void entry(
        tbb::concurrent_vector<world::LaborEntry>::iterator it  // NOLINT
    ) {
        comp_.posting_.myEntries_.emplace_back(&*it);
    }
    auto productPower() const -> double { return comp_.productPower_; }
    auto rng() -> pcg32& { return comp_.rng_; }
};

void jobEntry(
    JobEntryView                                 view,
    const int                                    id,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    const int                                    entryCnt = config::labor_supplier::jobEntryCnt
);
}  // namespace labor_supplier