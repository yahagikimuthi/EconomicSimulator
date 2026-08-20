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
#include <variant>

#include "core/assertion.hpp"
#include "setting.hpp"

namespace abm {
template <typename... Ts>
struct Overloaded final : Ts... {
    using Ts::operator()...;
};

template <typename T>
using RefWrap = std::reference_wrapper<T>;

class RandomGenerator final {
  public:
    [[nodiscard]] explicit RandomGenerator(const pcg32 rng) noexcept : rng_{rng} {}

    [[nodiscard]] auto rand(const double min = 0.0, const double limit = 1.0) noexcept -> double {
        auto dist = std::uniform_real_distribution<double>{min, limit};
        return dist(rng_);
    }

    [[nodiscard]] auto randInt(const int min, const int max) noexcept -> int {
        auto dist = std::uniform_int_distribution<int>{min, max};
        return dist(rng_);
    }

    [[nodiscard]] auto randNormal(
        const double mean = 0.0,
        const double div  = 1.0,
        const double min  = -std::numeric_limits<double>::infinity(),
        const double max  = std::numeric_limits<double>::infinity()
    ) noexcept -> double {
        auto       dist = std::normal_distribution<double>{mean, div};
        const auto out  = double{dist(rng_)};
        return std::clamp(out, min, max);
    }

    [[nodiscard]] auto rand(const int min, const int limit) noexcept -> int {
        auto dist = std::uniform_int_distribution<int>{min, limit};
        return dist(rng_);
    }

    template <std::ranges::range Container, typename Proj = std::identity>
        requires requires(Container container, Proj proj) {
            { std::invoke(proj, *container.begin()) } -> std::same_as<double>;
        }
    [[nodiscard]] auto discreteDistribution(
        Container&& container, const double total, Proj&& proj = {}
    ) noexcept -> decltype(auto) {
        ASSERT(total >= 0.0);
        const auto target     = rand(0.0, total);
        auto       currentCnt = 0.0;
        for (auto& elem : std::forward<Container>(container)) {
            currentCnt += std::invoke(std::forward<Proj>(proj), elem);
            if (currentCnt >= target) return elem;
        }
        ASSERT(false && "runtime error");
        std::unreachable();
    }

    template <std::ranges::range Container, typename Proj = std::identity>
        requires requires(Container container, Proj proj) {
            { std::invoke(proj, *container.begin()) } -> std::same_as<double>;
        }
    [[nodiscard]] auto discreteDistribution(Container&& container, Proj&& proj = {}) noexcept
        -> decltype(auto) {
        auto total = 0.0;
        for (const auto& elem : std::forward<Container>(container)) {
            const double weight = std::invoke(std::forward<Proj>(proj), elem);
            ASSERT(weight >= 0.0 && "weight is required >= 0");
            total += weight;
        }
        ASSERT(total >= 0.0 && "total is required >= 0");
        return discreteDistribution(std::forward<Container>(container), total, proj);
    }

    template <std::ranges::random_access_range Container>
        requires requires(Container& c, pcg32& rng) { std::ranges::shuffle(c, rng); }
    void shuffle(Container&& c) noexcept {
        std::ranges::shuffle(std::forward<Container>(c), rng_);
    }

    template <std::ranges::input_range Range, std::weakly_incrementable Out>
        requires requires(Range& range, Out outIt, int n, pcg32 rng) {
            std::ranges::sample(range, outIt, n, rng);
        }
    void sample(Range&& r, Out out, const int n) noexcept {
        std::ranges::sample(std::forward<Range>(r), out, n, rng_);
    }

    [[nodiscard]] auto makeUint64() noexcept -> std::uint64_t {
        return (static_cast<std::uint64_t>(rng_()) << 32) | rng_();
    }

    [[nodiscard]] auto random(const RandomParameter& param) noexcept -> double {
        return std::visit(
            Overloaded{
                [&] [[nodiscard]] (const UniformParameter<int>& uniformParam) -> double {
                    return randInt(
                        static_cast<int>(uniformParam.min), static_cast<int>(uniformParam.limit)
                    );
                },
                [&] [[nodiscard]] (const UniformParameter<double>& uniformParam) -> double {
                    return rand(uniformParam.min, uniformParam.limit);
                },
                [&] [[nodiscard]] (const NormalParameter& normalParam) -> double {
                    return randNormal(
                        normalParam.mean, normalParam.dev, normalParam.min, normalParam.max
                    );
                }
            },
            param
        );
    }

  private:
    pcg32 rng_;
};
}  // namespace abm