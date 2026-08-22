#include "abm.hpp"

auto main() -> int {
    auto engine = abm::Engine{1000, false};
    engine.run();
}