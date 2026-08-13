#pragma once

#include <concepts>

#include "core/values/common.hpp"

namespace abm::internal {
template <typename F>
concept AssetPlusFn = requires(F f, const Money money) {
    { f(money) } -> std::same_as<void>;
};
}  // namespace abm::internal