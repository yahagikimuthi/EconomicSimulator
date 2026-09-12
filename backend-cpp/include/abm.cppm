module;

#include "analysis/analysis.hpp"
#include "engine/engine.hpp"

export module abm;

namespace abm {
export void run(const int step, const bool isAnalysis = false) {
    auto engine = Engine{step};
    engine.run();
    if (isAnalysis) {
        analysis::analysisData();
    }
}
}  // namespace abm