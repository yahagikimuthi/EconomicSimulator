#pragma once

#include <concepts>
#include <functional>
#include <pcg_random.hpp>
#include <random>
#include <utility>

#include "config.hpp"
#include "core/base.hpp"

namespace abm::helper {
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

[[nodiscard]] constexpr auto makeSeed(pcg32& rng) -> std::uint64_t {
    return (static_cast<std::uint64_t>(rng()) << 32 | rng());
}

template <std::ranges::range Container, typename Proj = std::identity>
    requires requires(Container container, Proj proj) {
        { std::invoke(proj, *container.begin()) } -> std::same_as<double>;
        { *container.begin() } -> std::same_as<typename Container::value_type&>;
    }
[[nodiscard]] constexpr auto discreteDistribution(
    Container& container, std::uniform_random_bit_generator auto& rng, Proj proj = {}
) -> Container::value_type& {
    double total{0.0};
    for (const auto& elem : container) {
        const double weight = std::invoke(proj, elem);
        ASSERT(weight >= 0.0 && "weight is required >= 0");
        total += weight;
    }
    ASSERT(total >= 0.0 && "total is required >= 0");

    std::uniform_real_distribution<double> dist{0.0, total};

    const double target{rand(rng, 0.0, total)};
    double       currentCnt{0.0};
    for (auto& elem : container) {
        currentCnt += std::invoke(proj, elem);
        if (currentCnt >= target) {
            return elem;
        }
    }
    ASSERT(false && "runtime error");
    std::unreachable();
}
}  // namespace abm::helper