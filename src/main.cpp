#include "core/engine.hpp"

auto main() -> int {
    std::string  savePath{"../result"};
    core::Engine engine{3, savePath};
}