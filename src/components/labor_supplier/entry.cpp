#include "components/labor_supplier.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cassert>
#include <config.hpp>
#include <cstddef>
#include <functional>
#include <iterator>
#include <pcg_random.hpp>
#include <ranges>
#include <vector>

#include "world/message.hpp"

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
            return requestRef.get().wage;
        }
    );
}
}  // namespace

namespace labor_supplier {
auto LaborSupplier::shouldSearchJob() const -> bool {
    if (not employment_.isEmployed()) return true;
    return helper::rand(rng_) < jobSearchThreshold_;
}

void JobHunter::entry(
    const int                                    id,
    const LaborSupplier&                         laborSupplier,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    const int                                    entryCnt
) {
    isPosting_ = true;
    static thread_local std::vector<std::reference_wrapper<world::LaborRequest>> sampleRequests;
    pickSample(requestBox, sampleRequests, rng_);
    sortSample(sampleRequests);
    for (const auto i :
         std::views::iota(0UZ, std::min(static_cast<std::size_t>(entryCnt), requestBox.size()))) {
        auto& request = sampleRequests[i].get();
        if (not laborSupplier.isEligibleRequest(request)) continue;
        auto& entryBox = request.entryBox;
        auto  it{entryBox.emplace_back(id, laborSupplier.productPower(), request)};
        myEntries_.emplace_back(&*it);
    }
}

void LaborSupplier::entry(const int id, tbb::concurrent_vector<world::LaborRequest>& requestBox) {
    updateRosterEntry();
    if (not shouldSearchJob()) return;
    if (requestBox.empty()) return;
    jobHunter_.entry(id, *this, requestBox);
}
}  // namespace labor_supplier