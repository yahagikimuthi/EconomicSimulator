#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <pcg_random.hpp>
#include <ranges>

#include "components/labor_supplier/concepts.hpp"
#include "config.hpp"
#include "core/base.hpp"
#include "world/message.hpp"

namespace labor::supplier::internal {
inline void pickSample(
    tbb::concurrent_vector<world::LaborRequest>&              requestBox,
    std::vector<std::reference_wrapper<world::LaborRequest>>& sampleRequests,
    pcg32&                                                    rng,
    const int                                                 sampleCnt
) {
    const std::size_t k{std::min(static_cast<std::size_t>(sampleCnt), requestBox.size())};
    sampleRequests.clear();
    if (requestBox.size() <= static_cast<std::size_t>(sampleCnt)) {
        for (world::LaborRequest& request : requestBox)
            sampleRequests.emplace_back(std::ref(request));
        return;
    }
    std::ranges::sample(requestBox, std::back_inserter(sampleRequests), static_cast<int>(k), rng);
}

inline void sortSample(
    std::vector<std::reference_wrapper<world::LaborRequest>>& sortRequests, const int entryCnt
) {
    const std::size_t k{std::min(static_cast<std::size_t>(entryCnt), sortRequests.size())};
    std::ranges::partial_sort(
        sortRequests,
        sortRequests.begin() + static_cast<int>(k),
        std::ranges::greater{},
        [](const std::reference_wrapper<world::LaborRequest>& requestRef) -> double {
            return requestRef.get().wage.value();
        }
    );
}

[[nodiscard]] inline auto pickJobs(
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    pcg32&                                       rng,
    const int                                    sampleCnt,
    const int                                    entryCnt
) -> std::ranges::view auto {
    using Request = world::LaborRequest;
    static thread_local std::vector<std::reference_wrapper<Request>> sampleRequest;
    pickSample(requestBox, sampleRequest, rng, sampleCnt);
    sortSample(sampleRequest, entryCnt);
    return sampleRequest |
           std::views::transform([](std::reference_wrapper<Request> reqRef) -> Request& {
               return reqRef.get();
           });
}
}  // namespace labor::supplier::internal

namespace labor::supplier {
class JobHunter {
  public:
    JobHunter(const pcg32 rng) : rng_{rng} {}

    void entry(
        IsAlignedFn auto&&                           isAligned,
        MakeEntrySheetFn auto&&                      makeEntrySheet,
        tbb::concurrent_vector<world::LaborRequest>& requestBox,
        const int sampleCnt = config::labor_supplier::jobSampleCnt,
        const int entryCnt  = config::labor_supplier::jobEntryCnt
    ) PRE(entryCnt > 0) {
        using Request = world::LaborRequest;
        std::ranges::view auto alignedRequests{
            internal::pickJobs(requestBox, rng_, sampleCnt, entryCnt) |
            std::views::filter([&](const Request& req) -> bool { return isAligned(req); }) |
            std::views::take(entryCnt)
        };
        if (alignedRequests.empty()) return;
        isPosting_ = true;
        for (auto&& request : alignedRequests) myEntries_.emplace_back(makeEntrySheet(request));
    }

    void accept() {
        if (not isPosting_) return;
        std::optional<Entry&> acceptEntry{takeAcceptEntry()};
        if (not acceptEntry) return;
        acceptEntry->isAccept = true;
        acceptedEntry_        = acceptEntry;
    }
    void endStep() { myEntries_.clear(), acceptedEntry_.reset(), isPosting_ = false; }
    auto acceptedEntry() -> std::optional<world::LaborEntry&> { return acceptedEntry_; }

  private:
    using Entry = world::LaborEntry;
    auto takeAcceptEntry() -> std::optional<Entry&> {
        std::ranges::view auto offered{
            myEntries_ | std::views::filter([](const std::reference_wrapper<Entry> entry) -> bool {
                return entry.get().isOffer;
            }) |
            std::views::take(1)
        };
        if (offered.empty()) return std::nullopt;
        return offered.front().get();
    }

    mutable pcg32                                          rng_;
    std::vector<std::reference_wrapper<world::LaborEntry>> myEntries_;
    std::optional<world::LaborEntry&>                      acceptedEntry_{std::nullopt};
    bool                                                   isPosting_{false};

    friend class JobHunterTester;
};
}  // namespace labor::supplier