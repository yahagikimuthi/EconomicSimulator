#pragma once

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
    pipeline.execute();
}
}  // namespace analysis