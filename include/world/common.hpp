#pragma once

#include <tbb/concurrent_vector.h>

#include "core/setting.hpp"

namespace abm {
enum class EMarket : char { Labor, ConsumerGoods, ProductionGoods };

struct CensusDropBox final {
    tbb::concurrent_vector<double> firmAssets;
    tbb::concurrent_vector<double> postedEmployments;
    tbb::concurrent_vector<double> postedWages;
    tbb::concurrent_vector<double> employments;
    tbb::concurrent_vector<double> sumWages;
    tbb::concurrent_vector<double> prices;
    tbb::concurrent_vector<double> supplies;
    tbb::concurrent_vector<double> markups;
    tbb::concurrent_vector<double> inventories;

    tbb::concurrent_vector<double> hholdAssets;
    tbb::concurrent_vector<double> wages;

    [[nodiscard]] constexpr CensusDropBox() noexcept {
        const auto firmCnt  = setting::agent_count::bToCFirm + setting::agent_count::bToBFirm;
        const auto hholdCnt = setting::agent_count::hhold;

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