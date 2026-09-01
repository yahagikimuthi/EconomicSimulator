module;

#include "analysis/analysis.hpp"
#include "others/engine.hpp"

export module abm;

namespace abm {
export void run(const int step, const bool isAnalysis = false) {
    auto engine = Engine{Date{1U, 1U, step}};
    engine.run();
    if (isAnalysis) {
        analysis::analysisData();
    }
}
}  // namespace abm