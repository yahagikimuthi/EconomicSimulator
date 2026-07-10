#include "core/engine.hpp"

auto main() -> int {
    std::string  savePath{"../simulation_result.h5"};
    core::Engine engine{3, savePath};
    engine.run();
}