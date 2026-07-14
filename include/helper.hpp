#pragma once

#include <algorithm>
#include <cassert>
#include <functional>
#include <limits>
#include <pcg_random.hpp>
#include <random>
#include <utility>

#include "config.hpp"

namespace helper {
struct PCG32Seed {
    const std::uint64_t state;
    const std::uint64_t stream;
};

[[nodiscard]] constexpr auto generatePCG32Seed() -> PCG32Seed {
    if (not config::setting::useRuntimeRandomSeed) {
        return {
            .state = config::setting::fixedSeedState, .stream = config::setting::fixedSeedStream
        };
    }

    std::random_device  rd;
    const std::uint64_t state{(static_cast<std::uint64_t>(rd()) << 32) | rd()};
    const std::uint64_t stream{(static_cast<std::uint64_t>(rd()) << 32) | rd()};

    return {.state = state, .stream = stream};
}

[[nodiscard]] constexpr auto rand(pcg32& rng, const double min = 0.0, const double max = 1.0)
    -> double {
    std::uniform_real_distribution<double> dist{min, max};
    return dist(rng);
}

[[nodiscard]] constexpr auto randInt(pcg32& rng, const int min, const int max) -> int {
    std::uniform_int_distribution<int> dist{min, max};
    return dist(rng);
}

[[nodiscard]] constexpr auto randNormal(
    pcg32&       rng,
    const double mean   = 0.0,
    const double stddev = 1.0,
    const double min    = -std::numeric_limits<double>::infinity(),
    const double max    = std::numeric_limits<double>::infinity()
) -> double {
    std::normal_distribution<double> dist{mean, stddev};
    return std::clamp(dist(rng), min, max);
}

template <typename Container, typename Proj = std::identity>
[[nodiscard]] inline auto discreteDistribution(Container& container, pcg32& rng, Proj proj = {})
    -> Container::value_type& {
    double total{0.0};
    for (const auto& elem : container) {
        const double weight = std::invoke(proj, elem);
        assert(weight >= 0.0 && "weight is required >= 0");
        total += weight;
    }
    assert(total >= 0.0 && "total is required >= 0");

    std::uniform_real_distribution<double> dist{0.0, total};

    const double target{rand(rng, 0.0, total)};
    double       currentCnt{0.0};
    for (auto& elem : container) {
        currentCnt += std::invoke(proj, elem);
        if (currentCnt >= target) {
            return elem;
        }
    }
    assert(false && "runtime error");
    std::unreachable();
}
}  // namespace helper