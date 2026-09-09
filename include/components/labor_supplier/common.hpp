#pragma once

#include <concepts>

#include "world/labor.hpp"

namespace abm::labor::supplier {
template <typename F>
concept IsAlignedFn = requires(F f, const Request& request) {
    { f(request) } -> std::same_as<bool>;
};

template <typename F>
concept MakeEntrySheetFn = requires(F f, Request request) {
    { f(request) } -> std::same_as<Entry&>;
};
}  // namespace abm::labor::supplier