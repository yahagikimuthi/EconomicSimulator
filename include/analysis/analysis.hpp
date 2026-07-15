#pragma once

#include <numeric>
#include <string>

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
    Pipeline pipeline{};

    pipeline.requireData("prices");

    pipeline.registerMetric("cpi", [](const DataContext& ctx) -> double {
        const auto& prices = ctx.get("prices");
        return calcMean(prices);
    });

    pipeline.execute();
}
}  // namespace analysis