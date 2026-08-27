#pragma once

#include <optional>
#include <pcg_random.hpp>
#include <utility>

#include "components/common.hpp"
#include "core/setting.hpp"
#include "core/util.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "world/consumer_goods.hpp"

namespace abm::consumer_goods::demander {
using Request = ConsumerGoodsRequest;
using Entry   = ConsumerGoodsEntry;
using Market  = ConsumerGoodsMarket;

struct ATradeResult final {
    const Price         price;
    const GoodsQuantity purchaseAmount;
};

class Ledger final {
  public:
    [[nodiscard]] constexpr Ledger() noexcept = default;

    void reset() noexcept { purchasing_ = Money{0.0}; }
    void readTradeResult(const ATradeResult& result) noexcept {
        purchasing_ += result.price * result.purchaseAmount;
    }
    [[nodiscard]] auto purchased() const noexcept -> Money { return purchasing_; }

  private:
    Money purchasing_{0.0};
};

class Trader final {
  public:
    [[nodiscard]] explicit constexpr Trader(RandomGenerator& masterRng)
        : rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}} {}

    void request(
        const Money budget, Market& market, const int sampleCnt = setting::goodsSampleCnt
    ) noexcept {
        auto pickedEntry = market.pickEntry(sampleCnt, rng_);
        if (not myRequest_) return;
        myRequest_ = pickedEntry->request(budget / pickedEntry->price);
    }

    void afterTrade() noexcept {
        if (not myRequest_) return;
        ledger_.readTradeResult(
            {.price = myRequest_->entry.price, .purchaseAmount = myRequest_->tradeAmount()}
        );
    }

    [[nodiscard]] auto purchased() const noexcept -> Money { return ledger_.purchased(); }

    void reset() noexcept {
        ledger_.reset();
        myRequest_.reset();
    }

  private:
    Ledger                        ledger_;
    RandomGenerator               rng_;
    std::optional<const Request&> myRequest_{std::nullopt};
};

class ConsumerGoodsDemander final {
  public:
    [[nodiscard]] explicit constexpr ConsumerGoodsDemander(RandomGenerator& masterRng) noexcept
        : trader_{masterRng},
          mpc_{masterRng.random(setting::mpc)},
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
        trader_.request(budget, market, sampleCnt);
    }

    void afterTrade() noexcept { trader_.afterTrade(); }

    template <AssetMinusFn F>
    void endStep(F&& assetMinus) noexcept {
        const auto purchased = trader_.purchased();
        ASSERT(purchased >= Money{0.0});
        std::forward<F>(assetMinus)(purchased);
        trader_.reset();
    }

  private:
    [[nodiscard]] auto shouldPass(const Step step, const int frequency) const noexcept -> bool {
        return step % frequency != myPhase_;
    }

    Trader                     trader_;
    const double               mpc_;
    const Step                 myPhase_;
    static inline unsigned int instanceCnt_{};
};
}  // namespace abm::consumer_goods::demander

namespace abm {
using ConsumerGoodsDemander = consumer_goods::demander::ConsumerGoodsDemander;
}