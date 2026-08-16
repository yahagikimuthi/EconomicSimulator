#include "analysis/analysis.hpp"
#include "core/engine.hpp"

auto main() -> int {
    auto engine = abm::Engine{1000};
    engine.run();

    abm::analysis::analysisData();
}