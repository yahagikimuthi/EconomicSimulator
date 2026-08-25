module;

#include "engine.hpp"

export module abm;

namespace abm {
export void run(const int step, const bool isAnalysis = false) {
    auto engine = Engine{step, isAnalysis};
    engine.run();
}
}  // namespace abm