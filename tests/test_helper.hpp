#pragma once

#include <concepts>
#include <cstdint>
#include <random>

#include "doctest.h"

namespace test::helper {
inline auto makeSeed() -> std::uint64_t {
    static std::random_device rd{};
    return rd();
}

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <typename T>
concept ValueType = requires(T t) {
    { t.value() } -> Numeric;
};

auto equal(ValueType auto input, ValueType auto expect) -> bool {
    return input.value() == doctest::Approx(expect.value());
}
[[nodiscard]] auto isLessThanOrEq(ValueType auto input, ValueType auto max) -> bool {
    return (input <= max) or equal(input, max);
}
[[nodiscard]] auto isLargerThanOrEq(ValueType auto input, ValueType auto min) -> bool {
    return (input >= min) or equal(input, min);
}
}  // namespace test::helper