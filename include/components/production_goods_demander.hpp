#pragma once

#include <tbb/concurrent_vector.h>
#include <concepts>
#include <optional>
#include <pcg_random.hpp>
#include <utility>

#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "setting.hpp"
#include "util.hpp"
#include "world/goods.hpp"

namespace abm::production_goods::demander {

struct ATradeResult final {
    const Price         price;
    const GoodsQuantity tradeAmount;
};

struct TradeResult final {
    const Money         purchased;
    const GoodsQuantity tradeAmount;
};

class Ledger {
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
        : mpc_{masterRng.random(setting::mpc)} {}

    void request(
        const AgentID id,
        const Money   asset,
        Market&       market,
        const int     sampleCnt = setting::goodsSampleCnt
    ) noexcept {
        const auto budget = asset * mpc_;
        if (budget <= Money{0.0}) return;
        const auto pickedEntry = market.pickEntry(id, sampleCnt);
        if (not pickedEntry) return;
        myRequest_ = pickedEntry->request(budget / pickedEntry->price);
    }

    void afterTrade() noexcept {
        if (not myRequest_) return;
        ledger_.readTradeResult(
            {.price = myRequest_->entry.price, .tradeAmount = myRequest_->tradeAmount}
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
    std::optional<const Request&> myRequest_{std::nullopt};
    const double                  mpc_;
};
}  // namespace abm::production_goods::demander

namespace abm {
using ProductionGoodsDemander = production_goods::demander::ProductionGoodsDemander;
}