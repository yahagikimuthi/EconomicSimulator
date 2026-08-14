#pragma once

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <limits>
#include <pcg_random.hpp>
#include <random>

#include "core/values/common.hpp"

namespace abm::detail {
template <typename F>
concept AssetPlusFn = requires(F f, const Money money) {
    { f(money) } -> std::same_as<void>;
};

class [[nodiscard]] RandomGenerator {
  public:
    RandomGenerator(const pcg32 rng) : rng_{rng} {}

    auto rand(const double min = 0.0, const double limit = 1.0) -> double {
        std::uniform_real_distribution<double> dist(min, limit);
        return dist(rng_);
    }

    auto randNormal(
        const double mean = 0.0,
        const double div  = 1.0,
        const double min  = -std::numeric_limits<double>::infinity(),
        const double max  = std::numeric_limits<double>::infinity()
    ) -> double {
        std::normal_distribution<double> dist{mean, div};
        const double                     out{dist(rng_)};
        return std::clamp(out, min, max);
    }

    auto rand(const int min, const int limit) -> int {
        std::uniform_int_distribution<int> dist{min, limit};
        return dist(rng_);
    }

    auto makeSeed() -> std::uint64_t { return (static_cast<std::uint64_t>(rng_()) << 32) | rng_(); }

  private:
    pcg32 rng_;
};
}  // namespace abm::detail