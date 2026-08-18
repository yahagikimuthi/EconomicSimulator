#pragma once

#include <algorithm>

#include "analysis/context_task.hpp"
#include "config.hpp"
#include "pipeline.hpp"

namespace abm::analysis {
[[nodiscard]] auto calcMean(
    const std::vector<double>& container, const double nanToNum = 0.0
) noexcept -> double {
    if (container.empty()) return nanToNum;
    const auto sum = std::ranges::fold_left(container, 0.0, std::plus{});
    const auto n   = static_cast<double>(container.size());
    return sum / n;
}

void analysisData() {
    namespace name = config::save_name;
    auto pipeline  = Pipeline{};

    pipeline.requireData(name::firmAssets);
    pipeline.requireData(name::householdAssets);
    pipeline.requireData(name::prices);

    pipeline.registerMetric("avgFirmAssets", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::firmAssets));
    });
    pipeline.registerMetric("avgHholdAssets", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::householdAssets));
    });
    pipeline.registerMetric("avgPrices", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::prices));
    });

    pipeline.execute();
}
}  // namespace abm::analysis