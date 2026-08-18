#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <pcg_random.hpp>
#include <ranges>
#include <span>

#include "components/labor_supplier/concepts.hpp"
#include "config.hpp"
#include "core/base.hpp"
#include "util.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
inline void pickSample(
    std::span<RefWrap<LaborRequest>>    requestBox,
    std::vector<RefWrap<LaborRequest>>& sampleRequests,
    RandomGenerator&                    rng,
    const int                           sampleCnt
) {
    const auto k = std::size_t{std::min(static_cast<std::size_t>(sampleCnt), requestBox.size())};
    if (requestBox.size() <= static_cast<std::size_t>(sampleCnt)) {
        for (LaborRequest& request : requestBox) sampleRequests.emplace_back(std::ref(request));
        return;
    }
    rng.sample(requestBox, std::back_inserter(sampleRequests), static_cast<int>(k));
}

inline void sortSample(std::span<RefWrap<LaborRequest>> sortRequests, const int entryCnt) {
    const auto k = std::size_t{std::min(static_cast<std::size_t>(entryCnt), sortRequests.size())};
    std::ranges::partial_sort(
        sortRequests,
        sortRequests.begin() + static_cast<int>(k),
        std::ranges::greater{},
        [](const RefWrap<LaborRequest>& requestRef) -> double {
            return requestRef.get().wage.value();
        }
    );
}

[[nodiscard]] inline auto pickAndSortJobs(
    const std::span<RefWrap<LaborRequest>> requestBox,
    RandomGenerator&                       rng,
    const int                              sampleCnt,
    const int                              entryCnt
) -> std::ranges::view auto {
    using Request    = LaborRequest;
    using RequestRef = std::reference_wrapper<Request>;

    static thread_local auto sampleRequest = std::vector<RequestRef>{};
    sampleRequest.clear();
    pickSample(requestBox, sampleRequest, rng, sampleCnt);
    sortSample(sampleRequest, entryCnt);
    return sampleRequest |
           std::views::transform([](RequestRef reqRef) -> Request& { return reqRef.get(); });
}

class [[nodiscard]] MyEntries final {
  public:
    MyEntries() = default;
    void add(LaborEntry& entry) { entries_.emplace_back(std::ref(entry)); }
    void clear() { entries_.clear(); }

    [[nodiscard]] auto takeOfferedEntry() -> std::ranges::view auto {
        return entries_ | std::views::transform([](RefWrap<LaborEntry> ref) -> LaborEntry& {
                   return ref.get();
               }) |
               std::views::filter(&LaborEntry::isOffer);
    }

  private:
    std::vector<RefWrap<LaborEntry>> entries_;
};

class JobHunter final {
    using Entry = LaborEntry;

  public:
    [[nodiscard]] explicit JobHunter(const RandomGenerator rng) : rng_{rng} {}

    template <IsAlignedFn F1, MakeEntrySheetFn F2>
    void entry(
        F1&&                                   isAligned,
        F2&&                                   makeEntrySheet,
        const std::span<RefWrap<LaborRequest>> requestBox,
        const int                              sampleCnt = config::labor_supplier::jobSampleCnt,
        const int                              entryCnt  = config::labor_supplier::jobEntryCnt
    ) PRE(entryCnt > 0) {
        using Request = LaborRequest;
        std::ranges::view auto alignedRequests{
            pickAndSortJobs(requestBox, rng_, sampleCnt, entryCnt) |
            std::views::filter([&](const Request& req) -> bool {
                return std::forward<F1>(isAligned)(req);
            }) |
            std::views::take(entryCnt)
        };
        if (alignedRequests.empty()) return;
        isPosting_ = true;
        for (auto&& request : alignedRequests)
            myEntries_.add(std::forward<F2>(makeEntrySheet)(request));
    }

    void accept() {
        if (not isPosting_) return;
        const auto acceptEntry = takeAcceptEntry();
        if (not acceptEntry) return;
        acceptEntry->isAccept = true;
        acceptedEntry_        = acceptEntry;
    }

    void endStep() { myEntries_.clear(), acceptedEntry_.reset(), isPosting_ = false; }

    [[nodiscard]] auto huntedResult() -> std::optional<Entry&> { return acceptedEntry_; }

  private:
    [[nodiscard]] auto takeAcceptEntry() -> std::optional<Entry&> {
        auto offered = myEntries_.takeOfferedEntry() | std::views::take(1);
        if (offered.empty()) return std::nullopt;
        return offered.front();
    }

    mutable RandomGenerator rng_;
    MyEntries               myEntries_;
    std::optional<Entry&>   acceptedEntry_{std::nullopt};
    bool                    isPosting_{false};
};
}  // namespace abm::labor::supplier