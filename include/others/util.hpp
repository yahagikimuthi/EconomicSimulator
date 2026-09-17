#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <functional>
#include <iterator>
#include <limits>
#include <pcg_random.hpp>
#include <random>
#include <ranges>
#include <type_traits>
#include <utility>

#include "others/setting.hpp"
#include "others/type.hpp"

namespace abm {
template <typename... Ts>
struct Overloaded final : Ts... {
    using Ts::operator()...;
};

template <typename T>
using ref_w = std::reference_wrapper<T>;

constexpr void nothing([[maybe_unused]] auto&&... _) noexcept {}

template <typename T>
concept Numeric = std::is_floating_point_v<T> or std::is_integral_v<T>;

template <std::invocable<> F>
class ScopeExit final {
  public:
    [[nodiscard("RAIIオブジェクトであるため代入してください")]] explicit constexpr ScopeExit(
        F f
    ) noexcept
        : f_{std::move(f)} {}
    ScopeExit(const ScopeExit&) noexcept                    = delete;
    ScopeExit(ScopeExit&&) noexcept                         = delete;
    auto operator=(const ScopeExit&) noexcept -> ScopeExit& = delete;
    auto operator=(ScopeExit&&) noexcept -> ScopeExit&      = delete;
    ~ScopeExit() noexcept { std::invoke(f_); }

  private:
    F f_;
};

template <typename G>
    requires requires(G rng) {
        { rng() } -> std::convertible_to<u64>;
    }
[[nodiscard]] auto makeUint64(G&& rng) noexcept {
    return (static_cast<u64>(std::forward<G>(rng)()) << 32) | std::forward<G>(rng)();
}

namespace engine {
class Engine;
}

class RandomGenerator final {
    friend class engine::Engine;
    friend inline auto makeRng() noexcept -> RandomGenerator;

  public:
    [[nodiscard]] auto construct() noexcept -> RandomGenerator {
        return RandomGenerator{makeUint64(this->rng_), makeUint64(this->rng_)};
    }

    [[nodiscard]] auto rand(const f64 min = 0.0, const f64 limit = 1.0) noexcept -> f64 {
        auto dist = std::uniform_real_distribution<f64>{min, limit};
        return dist(rng_);
    }

    [[nodiscard]] auto randInt(const i32 min, const i32 max) noexcept -> i32 {
        auto dist = std::uniform_int_distribution<i32>{min, max};
        return dist(rng_);
    }

    [[nodiscard]] auto randNormal(
        const f64 mean = 0.0,
        const f64 div  = 1.0,
        const f64 min  = -std::numeric_limits<f64>::infinity(),
        const f64 max  = std::numeric_limits<f64>::infinity()
    ) noexcept -> f64 {
        auto       dist = std::normal_distribution<f64>{mean, div};
        const auto out  = dist(rng_);
        return std::clamp(out, min, max);
    }

    template <std::ranges::input_range Range, typename Proj = std::identity>
        requires requires(Range container, Proj proj) {
            { std::invoke(proj, *container.begin()) } -> std::same_as<f64>;
        }
    [[nodiscard]] auto discreteDistribution(
        Range&& container, const f64 total, Proj&& proj = {}
    ) noexcept -> decltype(auto) {
        assert(total > 0.0);
        const auto target     = rand(0.0, total);
        auto       currentCnt = 0.0;
        for (auto& elem : std::forward<Range>(container)) {
            currentCnt += std::invoke(proj, elem);
            if (currentCnt >= target) return elem;
        }
        assert(false);
        std::unreachable();
    }

    template <std::ranges::random_access_range Range>
        requires requires(Range& c, pcg32& rng) { std::ranges::shuffle(c, rng); }
    void shuffle(Range&& c) noexcept {
        std::ranges::shuffle(std::forward<Range>(c), rng_);
    }

    template <std::ranges::input_range Range, std::weakly_incrementable Out, std::integral N>
        requires requires(Range& range, Out outIt, N n, pcg32 rng) {
            std::ranges::sample(range, outIt, n, rng);
        }
    void sample(Range&& r, Out out, const N n) noexcept {
        std::ranges::sample(std::forward<Range>(r), out, static_cast<i32>(n), rng_);
    }

    [[nodiscard]] auto random(const RandomParameter& param) noexcept -> f64 {
        return param.visit(
            Overloaded{
                [&](const UniformParameter<i32>& uniformParam) noexcept -> f64 {
                    return randInt(uniformParam.min, uniformParam.limit);
                },
                [&](const UniformParameter<f64>& uniformParam) noexcept -> f64 {
                    return rand(uniformParam.min, uniformParam.limit);
                },
                [&](const NormalParameter& normalParam) noexcept -> f64 {
                    return randNormal(
                        normalParam.mean, normalParam.dev, normalParam.min, normalParam.max
                    );
                }
            }
        );
    }

  private:
    explicit RandomGenerator(const u64 state, const u64 stream) noexcept : rng_{state, stream} {}

    pcg32 rng_;
};
}  // namespace abm