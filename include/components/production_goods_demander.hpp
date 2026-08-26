#pragma once

#include <concepts>
#include <optional>
#include <pcg_random.hpp>
#include <utility>

#include "core/setting.hpp"
#include "core/util.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "core/values/integrate.hpp"
#include "world/production_goods.hpp"

namespace abm::production_goods::demander {

struct ATradeResult final {
    const Price         price;
    const GoodsQuantity tradeAmount;
};

struct TradeResult final {
    const Money         purchased;
    const GoodsQuantity tradeAmount;
};

class Ledger final {
  public:
    [[nodiscard]] constexpr Ledger() noexcept = default;

    void readTradeResult(const ATradeResult& result) noexcept {
        ASSERT(result.price >= Price{0.0});
        ASSERT(result.tradeAmount >= GoodsQuantity{0.0});

        purchasing_ += result.price * result.tradeAmount;
        purchaseAmount_ += result.tradeAmount;
    }

    void reset() noexcept {
        ASSERT(purchasing_ >= Money{0.0});
        ASSERT(purchaseAmount_ >= GoodsQuantity{0.0});
        purchasing_     = Money{0.0};
        purchaseAmount_ = GoodsQuantity{0.0};
    }

    [[nodiscard]] auto publishTradeResult() const noexcept -> TradeResult {
        return {.purchased = purchasing_, .tradeAmount = purchaseAmount_};
    }

  private:
    Money         purchasing_{0.0};
    GoodsQuantity purchaseAmount_{0.0};
};

template <typename F>
concept ReadResultFn = requires(F f, const TradeResult& result) {
    { f(result) } -> std::same_as<void>;
};

class ProductionGoodsDemander final {
    using Request = ProductionGoodsRequest;
    using Market  = ProductionGoodsMarket;

  public:
    [[nodiscard]] explicit constexpr ProductionGoodsDemander(RandomGenerator& masterRng) noexcept
        : rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}} {}

    void request(
        const AgentID       id,
        const Money         asset,
        const GoodsQuantity desiredPurchaseAmount,
        Market&             market,
        const int           sampleCnt = setting::goodsSampleCnt
    ) noexcept {
        if (desiredPurchaseAmount <= GoodsQuantity{0.0}) return;
        if (asset <= Money{0.0}) return;
        const auto pickedEntry = market.pickEntry(id, sampleCnt, rng_);
        if (not pickedEntry) return;
        const auto purchaseAmount = min(desiredPurchaseAmount, asset / pickedEntry->price);
        myRequest_                = pickedEntry->request(purchaseAmount);
    }

    void afterTrade() noexcept {
        if (not myRequest_) return;
        ledger_.readTradeResult(
            {.price = myRequest_->entry.price, .tradeAmount = myRequest_->tradeAmount()}
        );
    }

    template <ReadResultFn F>
    void endStep(F&& readResult) noexcept {
        const auto result = ledger_.publishTradeResult();
        ASSERT(result.purchased >= Money{0.0});
        ASSERT(result.tradeAmount >= GoodsQuantity{0.0});

        std::forward<F>(readResult)(result);
        reset();
    }

  private:
    void reset() noexcept {
        ledger_.reset();
        myRequest_.reset();
    }

    Ledger                        ledger_;
    RandomGenerator               rng_;
    std::optional<const Request&> myRequest_{std::nullopt};
};
}  // namespace abm::production_goods::demander

namespace abm {
using ProductionGoodsDemander = production_goods::demander::ProductionGoodsDemander;
}