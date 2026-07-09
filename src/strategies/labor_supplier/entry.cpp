#include "strategies/labor_supplier.hpp"

#include "config/init_setup.hpp"
#include "helper.hpp"

namespace labor_supplier {
namespace {
void pickSample(
    const tbb::concurrent_vector<world::LaborRequest>& requestBox,
    std::vector<std::size_t>&                          sampleIdxs,
    const int sampleCnt = config::labor_supplier::jobSampleCnt
) {
    assert(sampleCnt > 0 && "sample count is required > 0");
    sampleIdxs.resize(static_cast<std::size_t>(sampleCnt));
    const int lastIdx{static_cast<int>(requestBox.size()) - 1};

    // 求人数が探索数未満の場合、連番でインデックスを入れる
    if (requestBox.size() <= static_cast<std::size_t>(sampleCnt)) {
        std::ranges::iota(sampleIdxs, 0UZ);
        return;
    }

    // 探索数以上の場合は一様分布によりインデクスを入れる
    for (size_t i{0}; i < static_cast<std::size_t>(sampleCnt); ++i) {
        ACCESS(sampleIdxs, i) = static_cast<std::size_t>(helper::randInt(0, lastIdx));
    }
}

void sortSample(
    const tbb::concurrent_vector<world::LaborRequest>& requestBox,
    std::vector<std::size_t>&                          sortIdxs,
    const int entryCnt = config::labor_supplier::jobEntryCnt
) {
    assert(entryCnt > 0 && "entry count is required > 0");
    const std::size_t k{std::min(static_cast<std::size_t>(entryCnt), sortIdxs.size())};
    std::ranges::partial_sort(
        sortIdxs.begin(),
        sortIdxs.begin() + static_cast<int>(k),
        sortIdxs.end(),
        std::greater<std::size_t>{},
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
    static thread_local std::vector<std::size_t> pickSampleIdxs;
    pickSample(requestBox, pickSampleIdxs);
    sortSample(requestBox, pickSampleIdxs);

    const double productPower{view.productPower()};
    view.clearEntry();
    for (std::size_t i : pickSampleIdxs) {
        auto& request  = ACCESS(requestBox, i);
        auto& entryBox = request.entryBox_;
        view.entry(request, entryBox.emplace_back(id, productPower));
    }
}
}  // namespace labor_supplier