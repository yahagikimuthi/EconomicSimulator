#pragma once

#include <concepts>

#include "core/values/common.hpp"

namespace abm::internal {
template <typename T>
concept AssetPlusFn = requires(T t, const Money money) {
    { t(money) } -> std::same_as<void>;
};
}  // namespace abm::internal