#pragma once

#include <algorithm>
#include <concepts>
#include <limits>
#include <optional>
#include <pcg_random.hpp>
#include <utility>

#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "world/capital.hpp"

namespace abm::capital::demander {
struct Log final {
    const Money         purchase{std::numeric_limits<double>::epsilon()};
    const GoodsQuantity tradeAmount{std::numeric_limits<double>::infinity()};
};

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
    [[nodiscard]] Ledger() noexcept = default;

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

class CapitalDemander final {
    using Request = CapitalRequest;
    using Market  = CapitalMarket;

  public:
    [[nodiscard]] explicit CapitalDemander(RandomGenerator& masterRng) noexcept
        : rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}} {}

    [[nodiscard]] auto planAndRequestBudget(const GoodsQuantity desiredAmount) noexcept -> Money {
        const auto avgPrice = log_.purchase / log_.tradeAmount;
        purchaseAmountPlan_ = desiredAmount;
        budget_             = avgPrice * desiredAmount;
        return *budget_;
    }

    void revisePlan(const Money budget) noexcept { budget_ = budget; }

    void plan(const GoodsQuantity desiredAmount) noexcept { purchaseAmountPlan_ = desiredAmount; }

    void request(
        const AgentID id, Market& market, const int sampleCnt = setting::goodsSampleCnt
    ) noexcept {
        const auto pickedEntry = market.pickEntry(id, sampleCnt, rng_);
        if (not pickedEntry) return;
        const auto purchaseAmount = std::min(*purchaseAmountPlan_, *budget_ / pickedEntry->price);
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
    Log                           log_;
    std::optional<GoodsQuantity>  purchaseAmountPlan_{std::nullopt};
    std::optional<Money>          budget_{std::nullopt};
    std::optional<const Request&> myRequest_{std::nullopt};
};
}  // namespace abm::capital::demander

namespace abm {
using CapitalDemander = capital::demander::CapitalDemander;
}