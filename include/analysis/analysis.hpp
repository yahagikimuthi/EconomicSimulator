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

    analyzer.requireData(name::firmAssets);
    analyzer.requireData(name::postedEmployments);
    analyzer.requireData(name::postedWages);
    analyzer.requireData(name::employments);
    analyzer.requireData(name::sumWages);
    analyzer.requireData(name::prices);
    analyzer.requireData(name::supplies);
    analyzer.requireData(name::markups);
    analyzer.requireData(name::inventories);
    analyzer.requireData(name::householdAssets);
    analyzer.requireData(name::wages);

    analyzer.registerMetric("avgFirmAssets", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::firmAssets));
    });
    analyzer.registerMetric("avgPostedEmployments", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::postedEmployments));
    });
    analyzer.registerMetric("avgPostedWages", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::postedWages));
    });
    analyzer.registerMetric("avgEmployments", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::employments));
    });
    analyzer.registerMetric("avgSumWages", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::sumWages));
    });
    analyzer.registerMetric("avgPrices", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::prices));
    });
    analyzer.registerMetric("avgSupplies", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::supplies));
    });
    analyzer.registerMetric("avgMarkups", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::supplies));
    });
    analyzer.registerMetric("avgInventories", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::supplies));
    });
    analyzer.registerMetric("avgHholdAssets", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::supplies));
    });
    analyzer.registerMetric("avgWages", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::supplies));
    });

    analyzer.execute();
}
}  // namespace abm::analysis