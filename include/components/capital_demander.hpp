#pragma once

#include <algorithm>
#include <cassert>
#include <optional>

#include "components/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "world/base_goods.hpp"

namespace abm::capital::demander {

struct Log final {
    Money         purchase;
    GoodsQuantity tradeAmount;
};

class CapitalDemander final {
  public:
    explicit CapitalDemander(RandomGenerator& masterRng) noexcept
        : rng_{{masterRng.makeUint64(), masterRng.makeUint64()}},
          log_{
              .purchase    = Money{masterRng.random(setting::lastPurchase)},
              .tradeAmount = GoodsQuantity{masterRng.random(setting::lastTradeAmount)}
          } {}

    [[nodiscard]] auto planBudget(const GoodsQuantity desiredAmount) noexcept -> Budget {
        const auto avgPrice = log_.purchase / log_.tradeAmount;
        purchaseAmountPlan_ = desiredAmount;
        budget_             = static_cast<Budget>(avgPrice * desiredAmount);
        return *budget_;
    }

    void revisePlan(const Budget budget) noexcept { budget_ = budget; }

    template <TryWithdrawFn F>
    void request(
        const AgentID id,
        F&&           withdrawFn,
        Market&       market,
        const int     sampleCnt = setting::goodsSampleCnt
    ) noexcept {
        assert(budget_);
        assert(purchaseAmountPlan_);

        const auto budget       = *budget_;
        const auto purchasePlan = *purchaseAmountPlan_;

        budget_.reset();
        purchaseAmountPlan_.reset();

        const auto pickedEntry = market.pickEntry(id, sampleCnt, rng_);
        if (not pickedEntry) return;
        const auto payment =
            std::min(static_cast<Budget>(purchasePlan * pickedEntry->price), budget);
        const auto withdraw = std::forward<F>(withdrawFn)(payment);
        if (withdraw.isPositive()) myRequest_ = pickedEntry->request(withdraw);
    }

    template <DepositFn F1, AddGoodsFn F2>
    void afterTrade(F1&& depositFn, F2&& addCapitalFn) noexcept {
        if (not myRequest_) return;
        const auto remain = myRequest_->takeoutRemainPaid();
        std::forward<F1>(depositFn)(remain);
        const auto capital = myRequest_->takeoutTradeAmount();
        std::forward<F2>(addCapitalFn)(capital);
        if (capital.isPositive())
            log_ = {.purchase = myRequest_->payment - remain, .tradeAmount = capital};
    }

  private:
    RandomGenerator              rng_;
    Log                          log_;  // TODO ログ更新処理追加
    std::optional<GoodsQuantity> purchaseAmountPlan_{std::nullopt};
    std::optional<Budget>        budget_{std::nullopt};
    std::optional<Request&>      myRequest_{std::nullopt};
};
}  // namespace abm::capital::demander

namespace abm {
using CapitalDemander = capital::demander::CapitalDemander;
}