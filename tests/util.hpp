#pragma once

#include <random>

#include "others/type.hpp"
#include "others/util.hpp"

namespace abm {
[[nodiscard]] inline auto makeRng() noexcept -> RandomGenerator {
    static auto rd     = std::random_device{};
    const auto  state  = u64{(static_cast<u64>(rd()) << 32) | rd()};
    const auto  stream = u64{(static_cast<u64>(rd()) << 32) | rd()};
    return RandomGenerator{state, stream};
}
}  // namespace abm