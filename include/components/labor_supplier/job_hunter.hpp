#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <inplace_vector>
#include <optional>
#include <ranges>
#include <span>
#include <utility>

#include "components/labor_supplier/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {

template <std::size_t JobEntryCnt>
class MyEntries final {
  public:
    explicit MyEntries() noexcept = default;
    [[nodiscard]] auto takeOfferedEntry() noexcept -> auto {
        return entries_ |
               std::views::transform([](Ref<Entry> ref) noexcept -> Entry& { return ref.get(); }) |
               std::views::filter([](Entry& e) noexcept -> bool { return e.isOffer(); });
    }

    void add(Entry& entry) noexcept { entries_.unchecked_emplace_back(std::ref(entry)); }
    void clear() noexcept { entries_.clear(); }

  private:
    std::inplace_vector<Ref<Entry>, JobEntryCnt> entries_;
};

template <
    std::size_t JobSampleCnt = setting::jobSampleCnt,
    std::size_t JobEntryCnt  = setting::jobEntryCnt>
class JobHunter final {
  public:
    explicit JobHunter(RandomGenerator& masterRng) noexcept
        : rng_{{masterRng.makeUint64(), masterRng.makeUint64()}} {}

    void entry(
        const AgentID           id,
        IsAlignedFn auto&&      isAligned,
        MakeEntrySheetFn auto&& makeEntrySheet,
        Market&                 market
    ) noexcept {
        auto alignedRequests = pickAndSortJobs(id, market) |
                               std::views::filter([&](const Request& req) noexcept -> bool {
                                   return isAligned(req);
                               }) |
                               std::views::take(JobEntryCnt);
        if (alignedRequests.empty()) return;
        for (auto& request : alignedRequests) myEntries_.add(makeEntrySheet(request));
    }

    void accept() noexcept {
        const auto offeredEntry = takeOfferedEntry();
        if (not offeredEntry) return;
        offeredEntry->accept();
        acceptedEntry_ = offeredEntry;
    }

    [[nodiscard]] auto takeoutResult() noexcept -> std::optional<Entry&> {
        myEntries_.clear();
        return std::exchange(acceptedEntry_, std::nullopt);
    }

  private:
    [[nodiscard]] auto takeOfferedEntry() noexcept -> std::optional<Entry&> {
        auto offered = myEntries_.takeOfferedEntry() | std::views::take(1);
        if (offered.empty()) return std::nullopt;
        return offered.front();
    }

    [[nodiscard]] auto pickAndSortJobs(const AgentID id, Market& market) noexcept
        -> std::span<Ref<Request>> {
        static thread_local auto sampleRequest = std::inplace_vector<Ref<Request>, JobSampleCnt>{};
        sampleRequest.clear();
        market.pickRequest(id, sampleRequest, rng_);
        sortSample(sampleRequest);
        return sampleRequest;
    }

    static void sortSample(std::span<Ref<Request>> sortRequests) noexcept {
        const auto k = std::min(JobEntryCnt, sortRequests.size());
        std::ranges::partial_sort(
            sortRequests,
            sortRequests.begin() + static_cast<int>(k),
            std::ranges::greater{},
            [](const Ref<Request> requestRef) noexcept -> Wage { return requestRef.get().wage; }
        );
    }

    MyEntries<JobSampleCnt> myEntries_;
    RandomGenerator         rng_;
    std::optional<Entry&>   acceptedEntry_{std::nullopt};
};
}  // namespace abm::labor::supplier