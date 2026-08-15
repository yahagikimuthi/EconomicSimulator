#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <iterator>
#include <optional>
#include <pcg_random.hpp>
#include <ranges>

#include "components/labor_supplier/concepts.hpp"
#include "components/util.hpp"
#include "config.hpp"
#include "core/base.hpp"
#include "world/message.hpp"

namespace abm::labor::supplier {
inline void pickSample(
    LaborMarket::RequestBoxT&                          requestBox,
    std::vector<std::reference_wrapper<LaborRequest>>& sampleRequests,
    RandomGenerator&                                   rng,
    const int                                          sampleCnt
) {
    const std::size_t k{std::min(static_cast<std::size_t>(sampleCnt), requestBox.size())};
    sampleRequests.clear();
    if (requestBox.size() <= static_cast<std::size_t>(sampleCnt)) {
        for (LaborRequest& request : requestBox) sampleRequests.emplace_back(std::ref(request));
        return;
    }
    rng.sample(requestBox, std::back_inserter(sampleRequests), static_cast<int>(k));
}

inline void sortSample(
    std::vector<std::reference_wrapper<LaborRequest>>& sortRequests, const int entryCnt
) {
    const std::size_t k{std::min(static_cast<std::size_t>(entryCnt), sortRequests.size())};
    std::ranges::partial_sort(
        sortRequests,
        sortRequests.begin() + static_cast<int>(k),
        std::ranges::greater{},
        [](const std::reference_wrapper<LaborRequest>& requestRef) -> double {
            return requestRef.get().wage.value();
        }
    );
}

[[nodiscard]] inline auto pickJobs(
    LaborMarket::RequestBoxT requestBox,
    RandomGenerator&         rng,
    const int                sampleCnt,
    const int                entryCnt
) -> std::ranges::view auto {
    using Request = LaborRequest;
    static thread_local std::vector<std::reference_wrapper<Request>> sampleRequest;
    pickSample(requestBox, sampleRequest, rng, sampleCnt);
    sortSample(sampleRequest, entryCnt);
    return sampleRequest |
           std::views::transform([](std::reference_wrapper<Request> reqRef) -> Request& {
               return reqRef.get();
           });
}

class [[nodiscard]] MyEntries {
    template <typename T>
    using RefWrap = std::reference_wrapper<T>;

  public:
    MyEntries() = default;
    void add(LaborEntry& entry) { entries_.emplace_back(std::ref(entry)); }
    void clear() { entries_.clear(); }
    auto takeOfferedEntry() -> std::ranges::view auto {
        return entries_ | std::views::transform([](RefWrap<LaborEntry> ref) -> LaborEntry& {
                   return ref.get();
               }) |
               std::views::filter(&LaborEntry::isOffer);
    }

  private:
    std::vector<RefWrap<LaborEntry>> entries_;
};

class [[nodiscard]] JobHunter {
  public:
    JobHunter(const RandomGenerator rng) : rng_{rng} {}

    void entry(
        IsAlignedFn auto&&              isAligned,
        MakeEntrySheetFn auto&&         makeEntrySheet,
        const LaborMarket::RequestBoxT& requestBox,
        const int                       sampleCnt = config::labor_supplier::jobSampleCnt,
        const int                       entryCnt  = config::labor_supplier::jobEntryCnt
    ) PRE(entryCnt > 0) {
        using Request = LaborRequest;
        std::ranges::view auto alignedRequests{
            pickJobs(requestBox, rng_, sampleCnt, entryCnt) |
            std::views::filter([&](const Request& req) -> bool { return isAligned(req); }) |
            std::views::take(entryCnt)
        };
        if (alignedRequests.empty()) return;
        isPosting_ = true;
        for (auto&& request : alignedRequests) myEntries_.add(makeEntrySheet(request));
    }

    void accept() {
        if (not isPosting_) return;
        std::optional<Entry&> acceptEntry{takeAcceptEntry()};
        if (not acceptEntry) return;
        acceptEntry->isAccept = true;
        acceptedEntry_        = acceptEntry;
    }
    void endStep() { myEntries_.clear(), acceptedEntry_.reset(), isPosting_ = false; }
    auto acceptedEntry() -> std::optional<LaborEntry&> { return acceptedEntry_; }

  private:
    using Entry = LaborEntry;
    auto takeAcceptEntry() -> std::optional<Entry&> {
        std::ranges::view auto offered{myEntries_.takeOfferedEntry() | std::views::take(1)};
        if (offered.empty()) return std::nullopt;
        return offered.front();
    }

    mutable RandomGenerator    rng_;
    MyEntries                  myEntries_;
    std::optional<LaborEntry&> acceptedEntry_{std::nullopt};
    bool                       isPosting_{false};
};
}  // namespace abm::labor::supplier