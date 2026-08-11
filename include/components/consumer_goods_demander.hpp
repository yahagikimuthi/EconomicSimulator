#pragma once

#include "components/base_goods_demander.hpp"
#include "core/values/common.hpp"
#include "world/message.hpp"

namespace abm {
class [[nodiscard]] ConsumerGoodsDemander final : public BaseGoodsDemander<Market::consumerGoods> {
  public:
    ConsumerGoodsDemander(const pcg32 rng, const double mpc, const Step myPhase)
        : BaseGoodsDemander<Market::consumerGoods>::BaseGoodsDemander(rng, mpc),
          myPhase_{myPhase} {}

    void request(
        const Money asset, const Step step, tbb::concurrent_vector<ConsumerGoodsEntry>& entryBox
    ) {
        if (isPass(asset, step, entryBox)) return;
        const Money budget{calcBudget(asset)};
        if (budget <= Money{0.0}) return;
        isPosting_        = true;
        auto& pickedEntry = base_goods::demander::pickEntry(rng_, entryBox);
        myRequest_        = pickedEntry.request(GoodsQuantity{budget / pickedEntry.price});
    }

  private:
    auto isPass(
        const Money                                       asset,
        const Step                                        step,
        const tbb::concurrent_vector<ConsumerGoodsEntry>& entryBox
    ) const -> bool {
        if (asset <= Money{0.0}) return true;
        if (entryBox.empty()) return true;
        const Step dayOfWeek{step % config::goods_demander::maxPurchaseFrequency};
        return dayOfWeek != myPhase_;
    }

    const Step myPhase_;
};
}  // namespace abm