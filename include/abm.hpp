#pragma once

#include "analysis/analysis.hpp"
#include "others/engine.hpp"

namespace abm {
inline void run(const int step, const bool isAnalysis = false) {
    auto engine = Engine{Date{1, 1, step}};
    engine.run();
    if (isAnalysis) {
        analysis::analysisData();
    }
}
}  // namespace abm