#pragma once

#include <algorithm>
#include <cstddef>
#include <functional>
#include <inplace_vector>
#include <optional>
#include <ranges>
#include <span>

#include "others/setting.hpp"
#include "others/util.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
class MyEntries final {
  public:
    MyEntries() noexcept = default;
    [[nodiscard]] auto takeOfferedEntry() noexcept -> auto {
        return entries_ | std::views::transform([](RefWrap<Entry> ref) noexcept -> Entry& {
                   return ref.get();
               }) |
               std::views::filter([](Entry& e) noexcept -> bool { return e.isOffer(); });
    }

    void add(Entry& entry) noexcept { entries_.unchecked_emplace_back(std::ref(entry)); }
    void clear() noexcept { entries_.clear(); }

  private:
    std::inplace_vector<RefWrap<Entry>, setting::jobEntryCnt> entries_;
};

template <typename F>
concept IsAlignedFn = requires(F f, const Request& request) {
    { f(request) } -> std::same_as<bool>;
};

template <typename F>
concept MakeEntrySheetFn = requires(F f, Request request) {
    { f(request) } -> std::same_as<Entry&>;
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
        const int               entryCnt = setting::jobEntryCnt
    ) noexcept {
        if (acceptedEntry_) return;
        std::ranges::view auto alignedRequests{
            pickAndSortJobs(id, market, entryCnt) |
            std::views::filter([&](const Request& req) noexcept -> bool {
                return isAligned(req);
            }) |
            std::views::take(entryCnt)
        };
        if (alignedRequests.empty()) return;
        for (auto& request : alignedRequests) myEntries_.add(makeEntrySheet(request));
    }

    void accept() noexcept {
        if (acceptedEntry_) return;
        const auto offeredEntry = takeOfferedEntry();
        if (not offeredEntry) return;
        offeredEntry->accept();
        acceptedEntry_ = offeredEntry;
    }
    [[nodiscard]] auto huntedResult() noexcept -> std::optional<Entry&> { return acceptedEntry_; }

    void reset() noexcept { myEntries_.clear(), acceptedEntry_.reset(); }

  private:
    [[nodiscard]] auto takeOfferedEntry() noexcept -> std::optional<Entry&> {
        auto offered = myEntries_.takeOfferedEntry() | std::views::take(1);
        if (offered.empty()) return std::nullopt;
        return offered.front();
    }

    [[nodiscard]] auto pickAndSortJobs(
        const AgentID id, Market& market, const int entryCnt
    ) noexcept -> std::span<RefWrap<Request>> {
        static thread_local auto sampleRequest =
            std::inplace_vector<RefWrap<Request>, setting::jobSampleCnt>{};
        sampleRequest.clear();
        market.pickRequest(id, sampleRequest, rng_);
        sortSample(sampleRequest, entryCnt);
        return sampleRequest;
    }

    static void sortSample(std::span<RefWrap<Request>> sortRequests, const int entryCnt) noexcept {
        const auto k = std::min(static_cast<std::size_t>(entryCnt), sortRequests.size());
        std::ranges::partial_sort(
            sortRequests,
            sortRequests.begin() + static_cast<int>(k),
            std::ranges::greater{},
            [](const RefWrap<Request> requestRef) noexcept -> Wage { return requestRef.get().wage; }
        );
    }

    MyEntries             myEntries_;
    RandomGenerator       rng_;
    std::optional<Entry&> acceptedEntry_{std::nullopt};
};
}  // namespace abm::labor::supplier