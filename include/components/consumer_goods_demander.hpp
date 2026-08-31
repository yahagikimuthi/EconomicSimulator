#pragma once

#include <optional>
#include <pcg_random.hpp>
#include <utility>

#include "components/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "world/base_goods.hpp"

namespace abm::consumer_goods::demander {
using Request = base_goods::Request;
using Entry   = base_goods::Entry;
using Market  = base_goods::Market;

struct ATradeResult final {
    const Price         price;
    const GoodsQuantity purchaseAmount;
};

class Ledger final {
  public:
    Ledger() noexcept = default;

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
    explicit Trader(RandomGenerator& masterRng)
        : rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}} {}

    void request(
        const AgentID id,
        const Money   budget,
        Market&       market,
        const int     sampleCnt = setting::goodsSampleCnt
    ) noexcept {
        auto pickedEntry = market.pickEntry(id, sampleCnt, rng_);
        if (not myRequest_) return;
        myRequest_ = pickedEntry->request(budget);
    }

    void afterTrade() noexcept {
        if (not myRequest_) return;
        ledger_.readTradeResult(
            {.price = myRequest_->price(), .purchaseAmount = myRequest_->tradeAmount()}
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
    explicit ConsumerGoodsDemander(RandomGenerator& masterRng) noexcept
        : trader_{masterRng}, mpc_{masterRng.random(setting::mpc)} {}

    [[nodiscard]] auto planAndRequestBudget(const Money asset) noexcept -> Money {
        const auto requestBudget = asset * mpc_;
        budget_                  = requestBudget;
        return requestBudget;
    }

    void revisePlan(const Money budget) noexcept {
        ASSERT(budget_);
        ASSERT(budget_ <= budget);
        budget_ = budget;
    }

    void request(
        const AgentID id, Market& market, const int sampleCnt = setting::goodsSampleCnt
    ) noexcept {
        ASSERT(budget_);
        const auto budget = *budget_;
        if (budget <= Money{0.0}) return;
        trader_.request(id, budget, market, sampleCnt);
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
    Trader               trader_;
    const double         mpc_;
    std::optional<Money> budget_;
};
}  // namespace abm::consumer_goods::demander

namespace abm {
using ConsumerGoodsDemander = consumer_goods::demander::ConsumerGoodsDemander;
}