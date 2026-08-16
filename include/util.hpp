#pragma once

#include <algorithm>
#include <concepts>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <pcg_random.hpp>
#include <random>
#include <ranges>
#include <utility>

#include "core/base.hpp"
#include "core/values/common.hpp"

namespace abm {
template <typename F>
concept AssetPlusFn = requires(F f, const Money money) {
    { f(money) } -> std::same_as<void>;
};

class RandomGenerator {
  public:
    [[nodiscard]] RandomGenerator(const pcg32 rng) : rng_{rng} {}

    [[nodiscard]] auto rand(const double min = 0.0, const double limit = 1.0) -> double {
        std::uniform_real_distribution<double> dist(min, limit);
        return dist(rng_);
    }

    [[nodiscard]] auto randNormal(
        const double mean = 0.0,
        const double div  = 1.0,
        const double min  = -std::numeric_limits<double>::infinity(),
        const double max  = std::numeric_limits<double>::infinity()
    ) -> double {
        std::normal_distribution<double> dist{mean, div};
        const double                     out{dist(rng_)};
        return std::clamp(out, min, max);
    }

    [[nodiscard]] auto rand(const int min, const int limit) -> int {
        std::uniform_int_distribution<int> dist{min, limit};
        return dist(rng_);
    }

    template <std::ranges::range Container, typename Proj = std::identity>
        requires requires(Container container, Proj proj) {
            { std::invoke(proj, *container.begin()) } -> std::same_as<double>;
        }
    [[nodiscard]] auto discreteDistribution(Container&& container, Proj&& proj = {})
        -> decltype(auto) {
        double total{0.0};
        for (const auto& elem : std::forward<Container>(container)) {
            const double weight = std::invoke(std::forward<Proj>(proj), elem);
            ASSERT(weight >= 0.0 && "weight is required >= 0");
            total += weight;
        }
        ASSERT(total >= 0.0 && "total is required >= 0");

        const double target{rand(0.0, total)};
        double       currentCnt{0.0};
        for (auto& elem : std::forward<Container>(container)) {
            currentCnt += std::invoke(std::forward<Proj>(proj), elem);
            if (currentCnt >= target) {
                return elem;
            }
        }
        ASSERT(false && "runtime error");
        std::unreachable();
    }

    template <std::ranges::random_access_range Container>
        requires requires(Container& c, pcg32& rng) { std::ranges::shuffle(c, rng); }
    void shuffle(Container&& c) {
        std::ranges::shuffle(std::forward<Container>(c), rng_);
    }

    template <std::ranges::input_range Range, std::weakly_incrementable Out>
        requires requires(Range& range, Out outIt, int n, pcg32 rng) {
            std::ranges::sample(range, outIt, n, rng);
        }
    void sample(Range&& r, Out out, const int n) {
        std::ranges::sample(std::forward<Range>(r), out, n, rng_);
    }

    [[nodiscard]] auto makeUint64() -> std::uint64_t {
        return (static_cast<std::uint64_t>(rng_()) << 32) | rng_();
    }

  private:
    pcg32 rng_;
};
}  // namespace abm