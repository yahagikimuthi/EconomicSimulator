#pragma once

#include <concepts>

#include "world/message.hpp"

namespace abm::base_goods {
template <typename T>
concept EntryType = (std::same_as<T, ConsumerGoodsEntry> or std::same_as<T, ProductionGoodsEntry>);

template <typename T>
concept RequestType =
    (std::same_as<T, ConsumerGoodsRequest>) or std::same_as<T, ProductionGoodsRequest>;

}  // namespace abm::base_goods