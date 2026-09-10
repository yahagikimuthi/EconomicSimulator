#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <functional>
#include <iterator>
#include <limits>
#include <pcg_random.hpp>
#include <random>
#include <ranges>
#include <type_traits>
#include <utility>

#include "others/setting.hpp"

namespace abm {
template <typename... Ts>
struct Overloaded final : Ts... {
    using Ts::operator()...;
};

template <typename T>
using Ref = std::reference_wrapper<T>;

constexpr void nothing([[maybe_unused]] auto&&... _) noexcept {}

template <typename From, typename To>
concept IsCastable = requires(From from) { static_cast<To>(from); };

template <typename F>
    requires std::is_invocable_v<F>
class ScopeExit final {
  public:
    explicit ScopeExit(F f) noexcept : f_{f} {}
    ScopeExit(const ScopeExit&) noexcept                    = default;
    ScopeExit(ScopeExit&&) noexcept                         = default;
    auto operator=(const ScopeExit&) noexcept -> ScopeExit& = default;
    auto operator=(ScopeExit&&) noexcept -> ScopeExit&      = default;
    ~ScopeExit() noexcept { std::invoke(f_); }

  private:
    F f_;
};

template <typename F>
    requires std::is_invocable_v<F>
[[nodiscard]] auto makeScopeExit(F&& f) noexcept -> ScopeExit<F> {
    return ScopeExit<F>{std::forward<F>(f)};
}

struct PCG32Seed final {
    const std::uint64_t state;
    const std::uint64_t stream;
};

class RandomGenerator final {
  public:
    explicit constexpr RandomGenerator(const pcg32 rng) noexcept : rng_{rng} {}

    template <IsCastable<double> T = double>
        requires std::is_constructible_v<T, double> and (not std::is_integral_v<T>)
    [[nodiscard]] constexpr auto rand(const T min = 0.0, const T limit = 1.0) noexcept -> T {
        auto dist = std::uniform_real_distribution<double>{
            static_cast<double>(min), static_cast<double>(limit)
        };
        return T{static_cast<double>(dist(rng_))};
    }

    template <std::integral T>
    [[nodiscard]] constexpr auto rand(const T min = 0.0, const T limit = 1.0) noexcept -> double {
        return rand<double>(min, limit);
    }

    template <IsCastable<int> T>
        requires std::is_constructible_v<T, int>
    [[nodiscard]] constexpr auto randInt(const T min, const T max) noexcept -> T {
        auto dist =
            std::uniform_int_distribution<int>{static_cast<int>(min), static_cast<int>(max)};
        return T{dist(rng_)};
    }

    template <IsCastable<double> T = double>
        requires std::is_constructible_v<T, double>
    [[nodiscard]] constexpr auto randNormal(
        const T      mean = T{0.0},
        const double div  = 1.0,
        const double min  = -std::numeric_limits<double>::infinity(),
        const double max  = std::numeric_limits<double>::infinity()
    ) noexcept -> double {
        auto       dist = std::normal_distribution<double>{static_cast<double>(mean), div};
        const auto out  = dist(rng_);
        return T{std::clamp(out, min, max)};
    }

    template <std::ranges::range Container, IsCastable<double> T, typename Proj = std::identity>
        requires requires(Container container, Proj proj) {
            { std::invoke(proj, *container.begin()) } -> IsCastable<T>;
        } and std::totally_ordered<T> and std::is_constructible_v<T, double> and
                     requires(T t) { t += t; }
    [[nodiscard]] auto discreteDistribution(
        Container&& container, const T total, Proj&& proj = {}
    ) noexcept -> decltype(auto) {
        assert(total > T{0.0});
        const auto target     = rand<T>(T{0}, total);
        auto       currentCnt = T{0.0};
        for (auto& elem : std::forward<Container>(container)) {
            currentCnt += std::invoke(proj, elem);
            if (currentCnt >= target) return elem;
        }
        assert(false);
        std::unreachable();
    }

    template <std::ranges::random_access_range Container>
        requires requires(Container& c, pcg32& rng) { std::ranges::shuffle(c, rng); }
    void shuffle(Container&& c) noexcept {
        std::ranges::shuffle(std::forward<Container>(c), rng_);
    }

    template <std::ranges::input_range Range, std::weakly_incrementable Out, std::integral N>
        requires requires(Range& range, Out outIt, N n, pcg32 rng) {
            std::ranges::sample(range, outIt, n, rng);
        }
    void sample(Range&& r, Out out, const N n) noexcept {
        std::ranges::sample(std::forward<Range>(r), out, static_cast<int>(n), rng_);
    }

    [[nodiscard]] constexpr auto makeUint64() noexcept -> std::uint64_t {
        return (static_cast<std::uint64_t>(rng_()) << 32) | rng_();
    }

    [[nodiscard]] constexpr auto random(const RandomParameter& param) noexcept -> double {
        return param.visit(Overloaded{
            [&](const UniformParameter<int>& uniformParam) noexcept -> double {
                return randInt(uniformParam.min, uniformParam.limit);
            },
            [&](const UniformParameter<double>& uniformParam) noexcept -> double {
                return rand(uniformParam.min, uniformParam.limit);
            },
            [&](const NormalParameter& normalParam) noexcept -> double {
                return randNormal(
                    normalParam.mean, normalParam.dev, normalParam.min, normalParam.max
                );
            }
        });
    }

  private:
    pcg32 rng_;
};
}  // namespace abm