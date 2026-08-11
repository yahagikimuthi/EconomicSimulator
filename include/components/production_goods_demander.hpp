#pragma once

#include <tbb/concurrent_vector.h>

#include "components/base_goods_demander.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "world/message.hpp"

namespace production_goods::demander {
class [[nodiscard]] ProductionGoodsDemander final
    : public base_goods::demander::BaseGoodsDemander<Market::productionGoods> {
  public:
    using base_goods::demander::BaseGoodsDemander<Market::productionGoods>::BaseGoodsDemander;

    void request(const Money asset, tbb::concurrent_vector<ProductionGoodsEntry>& entryBox) {
        if (isPass(asset, entryBox)) return;
        const Money budget{calcBudget(asset)};
        if (budget <= Money{0.0}) return;
        isPosting_        = true;
        auto& pickedEntry = base_goods::demander::internal::pickEntry(rng_, entryBox);
        myRequest_        = pickedEntry.request(GoodsQuantity{budget / pickedEntry.price});
    }

  private:
    auto isPass(const Money asset, const tbb::concurrent_vector<ProductionGoodsEntry>& entryBox)
        const -> bool {
        if (asset <= Money{0.0}) return true;
        if (entryBox.empty()) return true;
        return false;
    }
};
}  // namespace production_goods::demander