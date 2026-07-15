#pragma once

#include <analysis/context_task.hpp>
#include <config.hpp>
#include <numeric>

#include "pipeline.hpp"

namespace analysis {
[[nodiscard]] auto calcMean(const std::vector<double>& container, const double nanToNum = 0.0)
    -> double {
    if (container.empty()) return nanToNum;
    const double sum{std::reduce(container.begin(), container.end(), 0.0)};
    const double n{static_cast<double>(container.size())};
    return sum / n;
}

void analysisData() {
    namespace name = config::save_name;
    Pipeline pipeline{};

    pipeline.requireData(name::firmAssets);
    pipeline.requireData(name::householdAssets);

    pipeline.registerMetric("avgFirmAssets", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::firmAssets));
    });
    pipeline.registerMetric("avgHholdAssets", [](const DataContext& ctx) -> double {
        return calcMean(ctx.get(name::householdAssets));
    });

    pipeline.execute();
}
}  // namespace analysis