#pragma once

#include <crow.h>

#include "engine/engine.hpp"
#include "others/type.hpp"

namespace abm {
inline void run(const i32 step) {
    auto engine = Engine{step};
    engine.run();
}
}  // namespace abm