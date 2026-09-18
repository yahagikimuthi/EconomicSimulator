#pragma once

#include <algorithm>
#include <cassert>
#include <optional>

#include "components/common.hpp"
#include "others/setting.hpp"
#include "others/type.hpp"
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
        : rng_{masterRng.construct()},
          log_{
              .purchase    = Money{masterRng.random(setting::lastPurchase)},
              .tradeAmount = GoodsQuantity{masterRng.random(setting::lastTradeAmount)}
          } {}

    [[nodiscard]] auto planBudget(const GoodsQuantity desiredAmount) noexcept -> Budget {
        assert(log_.tradeAmount.isPositive());
        assert(desiredAmount.isZeroOrMore());

        purchaseAmountPlan_ = std::min(desiredAmount, log_.tradeAmount * 2.0);
        budget_ = static_cast<Budget>(desiredAmount * (log_.purchase / log_.tradeAmount));
        assert(budget_->isZeroOrMore());
        return *budget_;
    }

    void revisePlan(const Budget budget) noexcept {
        assert(budget_);
        assert(budget.isZeroOrMore());
        budget_ = budget;
    }

    template <TryWithdrawFn F>
    void request(
        const AgentID id,
        F&&           withdrawFn,
        Market&       market,
        const i32     sampleCnt = setting::goodsSampleCnt
    ) noexcept {
        assert(budget_);
        assert(purchaseAmountPlan_);
        assert(budget_->isZeroOrMore());

        const auto budget       = *budget_;
        const auto purchasePlan = *purchaseAmountPlan_;

        budget_.reset();
        purchaseAmountPlan_.reset();

        auto pickedEntry = market.pickEntry(id, sampleCnt, rng_);
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
        if (capital.isPositive()) {
            log_ = {
                .purchase    = myRequest_->payment - remain + Money{global_setting::epsilon},
                .tradeAmount = capital
            };
        }
        myRequest_.reset();
    }

  private:
    RandomGenerator              rng_;
    Log                          log_;
    std::optional<GoodsQuantity> purchaseAmountPlan_{std::nullopt};
    std::optional<Budget>        budget_{std::nullopt};
    std::optional<Request&>      myRequest_{std::nullopt};
};
}  // namespace abm::capital::demander

namespace abm {
using CapitalDemander = capital::demander::CapitalDemander;
}