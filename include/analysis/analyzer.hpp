#pragma once

#include <string>

#include "pipeline.hpp"

namespace analysis {
void analysisData(std::string&& inputPath, std::string&& outputPath) {
    Pipeline pipeline{std::move(inputPath), std::move(outputPath)};

    pipeline.requireData("prices");

    pipeline.registerMetric("cpp", [](const DataContext& ctx) -> double {
        const auto& prices = ctx.get("prices");
        return prices[0];
    });

    pipeline.execute();
}
}  // namespace analysis