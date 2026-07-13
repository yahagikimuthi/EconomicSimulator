#include <print>

#include "core/engine.hpp"

auto main() -> int {
    core::Engine engine{1000, "../simulation_result.h5"};
    engine.run();

    std::println("finish");
}