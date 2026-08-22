#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <pcg_random.hpp>
#include <ranges>
#include <span>

#include "components/labor_supplier/common.hpp"
#include "core/setting.hpp"
#include "core/util.hpp"
#include "world/labor.hpp"

namespace abm::labor::supplier {
class MyEntries final {
  public:
    [[nodiscard]] MyEntries() noexcept = default;
    void add(Entry& entry) noexcept { entries_.emplace_back(std::ref(entry)); }
    void clear() noexcept { entries_.clear(); }

    [[nodiscard]] auto takeOfferedEntry() noexcept -> std::ranges::view auto {
        return entries_ |
               std::views::transform([](RefWrap<Entry> ref) -> Entry& { return ref.get(); }) |
               std::views::filter([](Entry& e) -> bool { return e.isOffer(); });
    }

  private:
    std::vector<RefWrap<Entry>> entries_;
};

class JobHunter final {
  public:
    [[nodiscard]] constexpr JobHunter() noexcept = default;

    template <IsAlignedFn F1, MakeEntrySheetFn F2>
    void entry(
        F1&&         isAligned,
        F2&&         makeEntrySheet,
        LaborMarket& market,
        const int    sampleCnt = setting::jobSampleCnt,
        const int    entryCnt  = setting::jobEntryCnt
    ) noexcept {
        if (acceptedEntry_) return;
        std::ranges::view auto alignedRequests{
            pickAndSortJobs(market, sampleCnt, entryCnt) |
            std::views::filter([&](const Request& req) -> bool {
                return std::forward<F1>(isAligned)(req);
            }) |
            std::views::take(entryCnt)
        };
        if (alignedRequests.empty()) return;
        for (auto&& request : alignedRequests)
            myEntries_.add(std::forward<F2>(makeEntrySheet)(request));
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

    [[nodiscard]] static auto pickAndSortJobs(
        LaborMarket& market, const int sampleCnt, const int entryCnt
    ) noexcept -> std::span<RefWrap<Request>> {
        static thread_local auto sampleRequest = std::vector<RefWrap<Request>>{};
        sampleRequest.clear();
        market.pickRequest(sampleRequest, sampleCnt);
        sortSample(sampleRequest, entryCnt);
        return sampleRequest;
    }

    static void sortSample(std::span<RefWrap<Request>> sortRequests, const int entryCnt) noexcept {
        const auto k =
            std::size_t{std::min(static_cast<std::size_t>(entryCnt), sortRequests.size())};
        std::ranges::partial_sort(
            sortRequests,
            sortRequests.begin() + static_cast<int>(k),
            std::ranges::greater{},
            [](const RefWrap<Request> requestRef) -> double {
                return requestRef.get().wage.value();
            }
        );
    }

    MyEntries             myEntries_;
    std::optional<Entry&> acceptedEntry_{std::nullopt};
};
}  // namespace abm::labor::supplier