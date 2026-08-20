#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>
#include <pcg_random.hpp>
#include <utility>

#include "components/others.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "setting.hpp"
#include "util.hpp"
#include "world/goods.hpp"

namespace abm::consumer_goods::demander {

struct ATradeResult {
    const Price         price;
    const GoodsQuantity purchaseAmount;
};

class Ledger {
  public:
    [[nodiscard]] Ledger() noexcept = default;

    void reset() noexcept { purchasing_ = Money{0.0}; }
    void readTradeResult(const ATradeResult& result) noexcept {
        purchasing_ += result.price * result.purchaseAmount;
    }
    [[nodiscard]] auto purchased() const noexcept -> Money { return purchasing_; }

  private:
    Money purchasing_{0.0};
};

class ConsumerGoodsDemander final {
    using Market  = ConsumerGoodsMarket;
    using Request = ConsumerGoodsRequest;
    using Entry   = ConsumerGoodsEntry;

  public:
    [[nodiscard]] explicit ConsumerGoodsDemander(RandomGenerator& masterRng) noexcept
        : mpc_{masterRng.random(setting::mpc)},
          myPhase_{instanceCnt_++ % setting::maxPurchaseFrequency} {}

    void request(
        const Money asset,
        const Step  step,
        Market&     market,
        const int   frequency = setting::maxPurchaseFrequency,
        const int   sampleCnt = setting::goodsSampleCnt
    ) noexcept {
        if (shouldPass(step, frequency)) return;
        const auto budget = asset * mpc_;
        if (budget <= Money{0.0}) return;
        auto pickedEntry = market.pickEntry(sampleCnt);
        if (not pickedEntry) return;
        myRequest_ = pickedEntry->request(budget / pickedEntry->price);
    }

    void afterTrade() noexcept {
        if (not myRequest_) return;
        ledger_.readTradeResult(
            {.price = myRequest_->entry.price, .purchaseAmount = myRequest_->tradeAmount}
        );
    }

    template <AssetMinusFn F>
    void endStep(F&& assetMinus) noexcept {
        ASSERT(ledger_.purchased() >= Money{0.0});
        std::forward<F>(assetMinus)(ledger_.purchased());
        reset();
    }

  private:
    [[nodiscard]] auto shouldPass(const Step step, const int frequency) const noexcept -> bool {
        return step % frequency != myPhase_;
    }

    void reset() noexcept {
        myRequest_.reset();
        ledger_.reset();
    }

    Ledger                        ledger_;
    std::optional<const Request&> myRequest_{std::nullopt};
    const double                  mpc_;
    const Step                    myPhase_;
    static inline int             instanceCnt_{};
};
}  // namespace abm::consumer_goods::demander

namespace abm {
using ConsumerGoodsDemander = consumer_goods::demander::ConsumerGoodsDemander;
}