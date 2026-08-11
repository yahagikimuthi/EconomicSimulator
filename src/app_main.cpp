#include "analysis/analysis.hpp"
#include "core/engine.hpp"

auto main() -> int {
    abm::Engine engine{1000};
    engine.run();

    abm::analysis::analysisData();
}