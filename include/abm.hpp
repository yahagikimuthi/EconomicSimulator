#pragma once

#include "engine.hpp"

namespace abm {
inline void run(const int step, const bool isAnalysis = false) {
    auto engine = Engine{step, isAnalysis};
    engine.run();
}
}  // namespace abm