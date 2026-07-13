#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <functional>
#include <limits>
#include <random>

#include "helper/random.hpp"

namespace helper {
inline static thread_local Pcg32 gen{generatePcg32()};  // NOLINT

[[nodiscard]] constexpr auto rand(Pcg32& genRef = gen) -> double {
    return static_cast<double>(genRef.next()) / 4294967296.0;  // 2^32
}

[[nodiscard]] constexpr auto rand(double min, double max) -> double {
    assert(min <= max && "min is required lower than last");
    return min + (rand() * (max - min));
}

[[nodiscard]] constexpr auto randInt(const int min, const int max) -> int {
    assert(min <= max && "first is required lower than last");
    const double randomNumber{min + (rand() * (max - min))};
    return static_cast<int>(std::floor(randomNumber));
}

[[nodiscard]] constexpr auto randNormal(
    const double mean   = 0.0,
    const double stddev = 1.0,
    const double min    = -std::numeric_limits<double>::infinity(),
    const double max    = std::numeric_limits<double>::infinity()
) -> double {
    // u1 は (0, 1] にして log(0) を回避する
    const double u1{1.0 - rand()};  // next_double() は [0,1) なので 1.0 から引いて (0,1] にする
    const double u2{rand()};
    const double z0{std::sqrt(-2.0 * std::log(u1)) * std::cos(2.0 * std::numbers::pi * u2)};
    const double out{mean + (z0 * stddev)};
    return std::clamp(out, min, max);
}

template <typename Container, typename Proj = std::identity>
[[nodiscard]] inline auto discreteDistribution(Container& container, Proj proj = {})
    -> Container::value_type& {
    double total{0.0};
    for (const auto& elem : container) {
        const double weight = std::invoke(proj, elem);
        assert(weight >= 0.0 && "weight is required >= 0");
        total += weight;
    }
    assert(total >= 0.0 && "total is required >= 0");

    std::uniform_real_distribution<double> dist{0.0, total};

    const double target{rand(0.0, total)};
    double       currentCnt{0.0};
    for (auto& elem : container) {
        currentCnt += std::invoke(proj, elem);
        if (currentCnt >= target) {
            return elem;
        }
    }
    assert(false && "runtime error");
}
}  // namespace helper