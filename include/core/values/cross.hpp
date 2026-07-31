#pragma once

#include "core/base.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"

[[nodiscard]] inline auto operator/(GoodsQuantity lhs, HeadCount rhs)
    -> double PRE(rhs.value() != 0.0) {
    return lhs.value() / rhs.value();
}