#include "analysis/analysis.hpp"
#include "core/engine.hpp"

auto main() -> int {
    Engine engine{1000};
    engine.run();

    analysis::analysisData();
}