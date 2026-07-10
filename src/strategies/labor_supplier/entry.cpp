#include "strategies/labor_supplier.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <numeric>
#include <random>
#include <ranges>

#include "config/init_setup.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace labor_supplier {
namespace {
void pickSample(
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    std::vector<world::LaborRequest*>            sampleRequestPtrs,
    const int                                    sampleCnt = config::labor_supplier::jobSampleCnt,
    std::mt19937&                                gen       = helper::gen
) {
    const std::size_t k{std::min(static_cast<std::size_t>(sampleCnt), requestBox.size())};
    sampleRequestPtrs.resize(k);

    if (requestBox.size() > static_cast<std::size_t>(sampleCnt)) {
        auto requestPtrs =
            requestBox | std::views::transform([](world::LaborRequest& r) -> world::LaborRequest* {
                return &r;
            });
        std::ranges::sample(requestPtrs, sampleRequestPtrs.begin(), static_cast<int>(k), gen);
        return;
    }

    for (std::size_t i{}; i < k; ++i) ACCESS(sampleRequestPtrs, i) = &ACCESS(requestBox, i);
}

void sortSample(
    const tbb::concurrent_vector<world::LaborRequest>& requestBox,
    std::vector<std::size_t>&                          sortIdxs,
    const int entryCnt = config::labor_supplier::jobEntryCnt
) {
    assert(entryCnt > 0 && "entry count is required > 0");
    const std::size_t k{std::min(static_cast<std::size_t>(entryCnt), sortIdxs.size())};
    std::ranges::partial_sort(
        sortIdxs,
        sortIdxs.begin() + static_cast<int>(k),
        std::ranges::greater{},
        [&requestBox](const std::size_t idx) -> double { return ACCESS(requestBox, idx).wage_; }
    );
}
}  // namespace

void jobEntry(
    JobEntryView view, const int id, tbb::concurrent_vector<world::LaborRequest>& requestBox
) {
    if (requestBox.empty()) {
        view.isPosting(false);
        return;
    }
    static thread_local std::vector<world::LaborRequest*> sampleRequestPtrs;
    pickSample(requestBox, sampleRequestPtrs);
    sortSample(requestBox, sampleRequestPtrs);

    const double productPower{view.productPower()};
    view.clearEntry();
    for (std::size_t i : sampleRequestPtrs) {
        auto& request  = ACCESS(requestBox, i);
        auto& entryBox = request.entryBox_;
        view.entry(request, entryBox.emplace_back(id, productPower));
    }
}
}  // namespace labor_supplier