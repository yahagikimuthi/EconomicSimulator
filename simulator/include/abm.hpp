#pragma once

#include "engine/engine.hpp"

namespace abm {
inline void run(const int step) {
    auto engine = Engine{step};
    engine.run();
}
}  // namespace abm