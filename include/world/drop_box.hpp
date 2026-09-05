#pragma once

#include <vector>

#include "others/setting.hpp"

namespace abm {
// 並列化する場合、tbb::concurrent_vectorにしなければならない
// しかし、HighFiveと互換性がないため、連続メモリコンテナで要素の追加を安全に行いたい
// 事前に容量を確保し、スレッドごとにインデックスを割り当てる方式を検討
struct CensusDropBox final {
    std::vector<double> firmAssets;
    std::vector<double> postedEmployments;
    std::vector<double> postedWages;
    std::vector<double> employments;
    std::vector<double> sumWages;
    std::vector<double> prices;
    std::vector<double> supplies;
    std::vector<double> markups;
    std::vector<double> inventories;

    std::vector<double> hholdAssets;
    std::vector<double> wages;

    explicit CensusDropBox() noexcept {
        namespace cnt       = global_setting::agent_count;
        const auto firmCnt  = cnt::capitalFirm + cnt::goodsFirm;
        const auto hholdCnt = cnt::hhold;

        firmAssets.reserve(firmCnt);
        postedEmployments.reserve(firmCnt);
        postedWages.reserve(firmCnt);
        employments.reserve(firmCnt);
        sumWages.reserve(firmCnt);
        prices.reserve(firmCnt);
        supplies.reserve(firmCnt);
        markups.reserve(firmCnt);
        inventories.reserve(firmCnt);

        hholdAssets.reserve(hholdCnt);
        wages.reserve(hholdCnt);
    }

    void clear() noexcept {
        firmAssets.clear();
        postedEmployments.clear();
        postedWages.clear();
        employments.clear();
        sumWages.clear();
        prices.clear();
        supplies.clear();
        markups.clear();
        inventories.clear();

        hholdAssets.clear();
        wages.clear();
    }
};
}  // namespace abm