#pragma once

#include <vector>

#include "core/setting.hpp"

namespace abm {
enum class Market : char { labor, consumerGoods, productionGoods };

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