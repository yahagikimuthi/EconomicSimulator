#pragma once

#include <algorithm>
#include <span>

#include "analysis/analyzer.hpp"
#include "analysis/context_task.hpp"
#include "setting.hpp"

namespace abm::analysis {
[[nodiscard]] constexpr auto calcMean(
    const std::span<const double> container, const double nanToNum = 0.0
) noexcept -> double {
    if (container.empty()) return nanToNum;
    const auto sum = std::ranges::fold_left(container, 0.0, std::plus{});
    const auto n   = static_cast<double>(container.size());
    return sum / n;
}

void analysisData() {
    namespace name = setting::save_name;
    auto analyzer  = Analyzer{};

    auto registerMean = [&](std::string&& outName, std::string_view readName) -> void {
        analyzer.requireData(readName);
        analyzer.registerMetric(std::move(outName), [&](const DataContext& ctx) -> double {
            return calcMean(ctx.get(readName));
        });
    };

    registerMean("avgFirmAssets", name::firmAssets);
    registerMean("avgPostedEmployments", name::postedEmployments);
    registerMean("avgPostedWages", name::postedWages);
    registerMean("avgEmployments", name::employments);
    registerMean("avgSumWages", name::sumWages);
    registerMean("avgPrices", name::prices);
    registerMean("avgSupplies", name::supplies);
    registerMean("avgMarkups", name::markups);
    registerMean("inventories", name::inventories);
    registerMean("avgHholdAssets", name::householdAssets);
    registerMean("avgWages", name::wages);
    analyzer.execute();
}
}  // namespace abm::analysis