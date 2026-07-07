#pragma once

#include <tbb/concurrent_vector.h>

#include "config/init_setup.hpp"
#include "core/forward.hpp"

namespace goods_demander {
struct PurchaseView;
[[nodiscard]] auto calcBudget(const PurchaseView& view, const double availableAsset) -> double;

[[nodiscard]] auto pickEntry(
    tbb::concurrent_vector<world::GoodsEntry>& entryBox,
    const int                                  sampleCnt = config::goods_demander::goodsSampleCnt
) -> world::GoodsEntry&;
}  // namespace goods_demander