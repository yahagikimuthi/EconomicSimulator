#pragma once

#include <cstddef>
#include <random>
#include <vector>

#include "helper.hpp"

namespace goods_supplier {
void shuffleIdx(
    const std::size_t         resizeNum,
    std::vector<std::size_t>& shuffleVec,
    std::mt19937&             gen = helper::gen
);
}