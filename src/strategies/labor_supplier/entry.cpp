#include "strategies/labor_supplier.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cassert>
#include <components/labor_supplier.hpp>
#include <cstddef>
#include <functional>
#include <iterator>
#include <pcg_random.hpp>
#include <ranges>
#include <vector>

#include "config.hpp"
#include "world/message.hpp"

namespace labor_supplier {
namespace {
void pickSample(
    tbb::concurrent_vector<world::LaborRequest>&              requestBox,
    std::vector<std::reference_wrapper<world::LaborRequest>>& sampleRequests,
    pcg32&                                                    rng,
    const int sampleCnt = config::labor_supplier::jobSampleCnt
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

void sortSample(
    std::vector<std::reference_wrapper<world::LaborRequest>>& sortRequests,
    const int entryCnt = config::labor_supplier::jobEntryCnt
) {
    const std::size_t k{std::min(static_cast<std::size_t>(entryCnt), sortRequests.size())};
    std::ranges::partial_sort(
        sortRequests,
        sortRequests.begin() + static_cast<int>(k),
        std::ranges::greater{},
        [](const std::reference_wrapper<world::LaborRequest>& requestRef) -> double {
            return requestRef.get().wage_;
        }
    );
}
}  // namespace

struct [[nodiscard]] CheckEmployingView final : BaseView<Component> {
    auto firmIdx() const -> std::size_t { return comp_.contraction_.contractFirmIdx_; }
    auto myIdx() const -> std::size_t { return comp_.contraction_.myRosterEntryIdx_; }
};

[[nodiscard]] auto isEmployingCheck(
    const CheckEmployingView& view, const std::span<const world::CompanyBoard> companyBoards
) -> bool {
    return companyBoards[view.firmIdx()].roster_[view.myIdx()].isLaidOff;
}

void jobEntry(
    JobEntryView                                 view,
    const int                                    id,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    const int                                    entryCnt
) {
    if (requestBox.empty()) {
        view.isPosting(false);
        return;
    }
    view.isPosting(true);
    static thread_local std::vector<std::reference_wrapper<world::LaborRequest>> sampleRequests;
    pickSample(requestBox, sampleRequests, view.rng());
    sortSample(sampleRequests);

    const double productPower{view.productPower()};

    for (const auto i : std::views::iota(0UZ, static_cast<std::size_t>(entryCnt))) {
        auto& request  = sampleRequests[i].get();
        auto& entryBox = request.entryBox_;
        view.entry(request, entryBox.emplace_back(id, productPower));
    }
}
}  // namespace labor_supplier