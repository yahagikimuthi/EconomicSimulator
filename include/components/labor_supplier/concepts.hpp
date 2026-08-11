#pragma once

#include <concepts>

#include "world/message.hpp"

namespace labor::supplier {
template <typename T>
concept IsAlignedFn = requires(T t, LaborRequest request) {
    { t(request) } -> std::same_as<bool>;
};

template <typename T>
concept MakeEntrySheetFn = requires(T t, LaborRequest request) {
    { t(request) } -> std::same_as<LaborEntry&>;
};
}  // namespace labor::supplier