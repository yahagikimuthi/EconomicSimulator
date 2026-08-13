#pragma once

#include <concepts>

#include "world/message.hpp"

namespace abm::labor::supplier {
template <typename F>
concept IsAlignedFn = requires(F f, LaborRequest request) {
    { f(request) } -> std::same_as<bool>;
};

template <typename T>
concept MakeEntrySheetFn = requires(T t, LaborRequest request) {
    { t(request) } -> std::same_as<LaborEntry&>;
};
}  // namespace abm::labor::supplier