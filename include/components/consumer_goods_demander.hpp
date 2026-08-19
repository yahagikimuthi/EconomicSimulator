#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>
#include <pcg_random.hpp>

#include "config.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"
#include "world/goods.hpp"

namespace abm::consumer_goods::demander {
class ConsumerGoodsDemander final {
    using Market  = ConsumerGoodsMarket;
    using Request = ConsumerGoodsRequest;
    using Entry   = ConsumerGoodsEntry;

  public:
    [[nodiscard]] explicit ConsumerGoodsDemander(RandomGenerator& masterRng) noexcept;

    void request(
        const Money asset,
        const Step  step,
        Market&     market,
        const int   frequency = config::goods_demander::maxPurchaseFrequency,
        const int   sampleCnt = config::goods_demander::goodsSampleCnt
    ) noexcept {
        if (shouldPass(step, frequency)) return;
        const auto budget = asset * mpc_;
        if (budget <= Money{0.0}) return;
        auto pickedEntry = market.pickEntry(sampleCnt);
        if (not pickedEntry) return;
        myRequest_ = pickedEntry->request(budget / pickedEntry->price);
    }

    void afterTrade() noexcept {}

    void endStep() noexcept { myRequest_.reset(); }

  private:
    [[nodiscard]] auto shouldPass(const Step step, const int frequency) const noexcept -> bool {
        return step % frequency != myPhase_;
    }

    void reset() noexcept { myRequest_.reset(); }

    std::optional<const Request&> myRequest_{std::nullopt};
    const double                  mpc_;
    const Step                    myPhase_;
};
}  // namespace abm::consumer_goods::demander

namespace abm {
using ConsumerGoodsDemander = consumer_goods::demander::ConsumerGoodsDemander;
}