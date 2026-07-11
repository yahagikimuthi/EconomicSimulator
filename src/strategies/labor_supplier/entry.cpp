#include "strategies/labor_supplier.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <functional>
#include <iterator>
#include <random>
#include <ranges>

#include "config.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace labor_supplier {
namespace {
void pickSample(
    tbb::concurrent_vector<world::LaborRequest>&              requestBox,
    std::vector<std::reference_wrapper<world::LaborRequest>>& sampleRequests,
    const int     sampleCnt = config::labor_supplier::jobSampleCnt,
    std::mt19937& gen       = helper::gen
) {
    const std::size_t k{std::min(static_cast<std::size_t>(sampleCnt), requestBox.size())};
    sampleRequests.clear();

    if (requestBox.size() <= static_cast<std::size_t>(sampleCnt)) {
        for (const auto i : std::views::iota(0UZ, k)) sampleRequests[i] = std::ref(requestBox[i]);
        return;
    }

    std::ranges::sample(requestBox, std::back_inserter(sampleRequests), static_cast<int>(k), gen);
}

void sortSample(
    std::vector<std::reference_wrapper<world::LaborRequest>>& sortIdxs,
    const int entryCnt = config::labor_supplier::jobEntryCnt
) {
    const std::size_t k{std::min(static_cast<std::size_t>(entryCnt), sortIdxs.size())};
    std::ranges::partial_sort(
        sortIdxs,
        sortIdxs.begin() + static_cast<int>(k),
        std::ranges::greater{},
        [](const std::reference_wrapper<world::LaborRequest>& requestRef) -> double {
            return requestRef.get().wage_;
        }
    );
}
}  // namespace

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
    pickSample(requestBox, sampleRequests);
    sortSample(sampleRequests);

    const double productPower{view.productPower()};

    for (const auto i : std::views::iota(0UZ, static_cast<std::size_t>(entryCnt))) {
        auto& request  = sampleRequests[i].get();
        auto& entryBox = request.entryBox_;
        view.entry(request, entryBox.emplace_back(id, productPower));
    }
}
}  // namespace labor_supplier