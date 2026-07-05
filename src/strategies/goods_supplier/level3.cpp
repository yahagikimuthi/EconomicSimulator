#include "strategies/goods_supplier/level3.hpp"

#include <algorithm>
#include <cstddef>
#include <random>
#include <vector>

namespace goods_supplier {
void shuffleIdx(
    const std::size_t resizeNum, std::vector<std::size_t>& shuffleVec, std::mt19937& gen
) {
    shuffleVec.resize(resizeNum);
    std::ranges::iota(shuffleVec, 0UZ);
    std::ranges::shuffle(shuffleVec, gen);
}
}  // namespace goods_supplier