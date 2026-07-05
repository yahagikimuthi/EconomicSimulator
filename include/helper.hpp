#pragma once

#pragma once

#include <algorithm>
#include <functional>
#include <limits>
#include <random>

#include "config/contract.hpp"

namespace helper {
inline static thread_local std::random_device rd;       // NOLINT
inline static thread_local std::mt19937       gen{42};  // NOLINT

[[nodiscard]] inline auto rand(
    const double first = 0.0, const double last = 1.0, std::mt19937& genRef = gen
) -> double {
    REQUIRE(first <= last, "first is required lower than last");
    std::uniform_real_distribution<double> dist{first, last};
    return dist(genRef);
}

[[nodiscard]] inline auto randInt(const int first, const int last, std::mt19937& genRef = gen) {
    REQUIRE(first <= last, "first is required lower than last");
    std::uniform_int_distribution<int> dist{first, last};
    return dist(genRef);
}

[[nodiscard]] inline auto randNormal(
    const double  mean   = 0.0,
    const double  std    = 1.0,
    const double  min    = -std::numeric_limits<double>::infinity(),
    const double  max    = std::numeric_limits<double>::infinity(),
    std::mt19937& genRef = gen
) -> double {
    REQUIRE(min <= max, "first is required lower than last");
    REQUIRE(std > 0.0, "std is required >= 0");
    std::normal_distribution<double> dist{mean, std};

    return std::clamp(dist(genRef), min, max);
}

template <typename Container, typename Proj = std::identity>
[[nodiscard]] inline auto discreteDistribution(
    Container& container, Proj proj = {}, std::mt19937& genRef = gen
) -> Container::value_type& {
    double total{0.0};
    for (const auto& elem : container) {
        const double weight = std::invoke(proj, elem);
        REQUIRE(weight >= 0.0, "weight is required >= 0");
        total += weight;
    }
    REQUIRE(total >= 0, "total is required >= 0");

    std::uniform_real_distribution<double> dist{0.0, total};

    const double target = dist(genRef);
    double       currentCnt{0.0};
    for (auto& elem : container) {
        currentCnt += std::invoke(proj, elem);
        if (currentCnt >= target) {
            return elem;
        }
    }

    ASSERT(false, "runtime error");
}
}  // namespace helper