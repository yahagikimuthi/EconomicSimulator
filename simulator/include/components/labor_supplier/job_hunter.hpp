#pragma once

#include <algorithm>
#include <functional>
#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

#include "components/labor_supplier/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {

class MyEntries final {
  public:
    explicit MyEntries() noexcept = default;

    [[nodiscard]] auto takeFirstOfferedEntry() noexcept -> std::optional<Entry&> {
        auto out = std::ranges::find_if(entries_, &Entry::isOffer);
        if (out == entries_.end()) return std::nullopt;
        return out->get();
    }

    void add(Entry& entry) noexcept { entries_.emplace_back(std::ref(entry)); }
    void clear() noexcept { entries_.clear(); }

  private:
    std::vector<Ref<Entry>> entries_;
};

class JobHunter final {
  public:
    explicit JobHunter(RandomGenerator& masterRng) noexcept
        : rng_{{masterRng.makeUint64(), masterRng.makeUint64()}} {}

    void entry(
        const AgentID           id,
        IsAlignedFn auto&&      isAligned,
        MakeEntrySheetFn auto&& makeEntrySheet,
        Market&                 market,
        const int               sampleCnt = setting::jobSampleCnt,
        const int               entryCnt  = setting::jobEntryCnt
    ) noexcept {
        auto alignedRequests = pickAndSortJobs(id, sampleCnt, entryCnt, market) |
                               std::views::filter([&](const Request& req) noexcept -> bool {
                                   return isAligned(req);
                               }) |
                               std::views::take(entryCnt);
        if (alignedRequests.empty()) return;
        for (auto& request : alignedRequests) myEntries_.add(makeEntrySheet(request));
    }

    void accept() noexcept {
        auto offeredEntry = myEntries_.takeFirstOfferedEntry();
        if (not offeredEntry) return;
        offeredEntry->accept();
        acceptedEntry_ = offeredEntry;
    }

    [[nodiscard]] auto takeoutResult() noexcept -> std::optional<Entry&> {
        myEntries_.clear();
        return std::exchange(acceptedEntry_, std::nullopt);
    }

  private:
    [[nodiscard]] auto pickAndSortJobs(
        const AgentID id, const int sampleCnt, const int entryCnt, Market& market
    ) noexcept -> std::span<Ref<Request>> {
        static thread_local auto sampleRequest = std::vector<Ref<Request>>{};
        sampleRequest.clear();
        market.pickRequest(id, sampleRequest, sampleCnt, rng_);
        sortSample(sampleRequest, entryCnt);
        return sampleRequest;
    }

    static void sortSample(std::span<Ref<Request>> sortRequests, const int entryCnt) noexcept {
        const auto k = std::min(entryCnt, static_cast<int>(sortRequests.size()));
        std::ranges::partial_sort(
            sortRequests,
            sortRequests.begin() + k,
            std::ranges::greater{},
            [](const Ref<Request> requestRef) noexcept -> Wage { return requestRef.get().wage; }
        );
    }

    MyEntries             myEntries_;
    RandomGenerator       rng_;
    std::optional<Entry&> acceptedEntry_{std::nullopt};
};
}  // namespace abm::labor::supplier