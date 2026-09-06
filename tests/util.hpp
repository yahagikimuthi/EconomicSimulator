#pragma once

#include <cstdint>
#include <random>

#include "others/util.hpp"

namespace abm {
[[nodiscard]] inline auto makeRng() noexcept -> RandomGenerator {
    static auto rd     = std::random_device{};
    const auto  state  = std::uint64_t{(static_cast<std::uint64_t>(rd()) << 32) | rd()};
    const auto  stream = std::uint64_t{(static_cast<std::uint64_t>(rd()) << 32) | rd()};
    return RandomGenerator{{state, stream}};
}
}  // namespace abm