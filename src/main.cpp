#include "core/engine.hpp"

auto main() -> int {
    core::Engine engine{10, "../simulation_result.h5"};
    engine.run();
}