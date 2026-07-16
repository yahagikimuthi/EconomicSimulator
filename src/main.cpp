#include "analysis/analysis.hpp"
#include "core/engine.hpp"

auto main() -> int {
    core::Engine engine{1000};
    engine.run();

    analysis::analysisData();
}