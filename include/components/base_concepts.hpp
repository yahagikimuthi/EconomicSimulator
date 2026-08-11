#pragma once

#include <concepts>

#include "world/message.hpp"

namespace base_goods {
template <typename T>
concept EntryType =
    (std::same_as<T, world::ConsumerGoodsEntry> or std::same_as<T, world::ProductionGoodsEntry>);

template <typename T>
concept RequestType = (std::same_as<T, world::ConsumerGoodsRequest>) or
                      std::same_as<T, world::ProductionGoodsRequest>;

}  // namespace base_goods