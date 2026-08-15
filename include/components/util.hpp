#pragma once

#include <oneapi/tbb/concurrent_vector.h>
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

    template <std::ranges::range Container, typename Proj = std::identity>
        requires requires(Container container, Proj proj) {
            { std::invoke(proj, *container.begin()) } -> std::same_as<double>;
            { *container.begin() } -> std::same_as<typename Container::value_type&>;
        }
    auto discreteDistribution(Container& container, Proj proj = {}) -> Container::value_type& {
        double total{0.0};
        for (const auto& elem : container) {
            const double weight = std::invoke(proj, elem);
            ASSERT(weight >= 0.0 && "weight is required >= 0");
            total += weight;
        }
        ASSERT(total >= 0.0 && "total is required >= 0");

        std::uniform_real_distribution<double> dist{0.0, total};

        const double target{rand(0.0, total)};
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

    auto makeUint64() -> std::uint64_t {
        return (static_cast<std::uint64_t>(rng_()) << 32) | rng_();
    }

    auto get() -> pcg32& { return rng_; }

  private:
    pcg32 rng_;
};
}  // namespace abm::detail