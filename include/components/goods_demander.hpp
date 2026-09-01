#pragma once

#include <concepts>
#include <optional>
#include <utility>

#include "components/finance/others_finance.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "world/base_goods.hpp"

namespace abm::goods::demander {

template <typename F>
concept TryWithdrawFn = requires(F f, Budget budget) {
    { f(budget) } -> std::same_as<Money>;
};

class Trader final {
  public:
    explicit Trader(RandomGenerator& masterRng)
        : rng_{{masterRng.makeUint64(), masterRng.makeUint64()}} {}

    void request(
        const AgentID        id,
        const Budget         budget,
        TryWithdrawFn auto&& withdrawFn,
        Market&              market,
        const int            sampleCnt = setting::goodsSampleCnt
    ) noexcept {
        auto pickedEntry = market.pickEntry(id, sampleCnt, rng_);
        if (not myRequest_) return;
        myRequest_ = pickedEntry->request(withdrawFn(budget));
    }

    void afterTrade(HHoldFinance& finance) noexcept {
        if (not myRequest_) return;
        const auto remain = myRequest_->remainPaid();
        finance.assetPlus(remain);
    }

    void reset() noexcept { myRequest_.reset(); }

  private:
    RandomGenerator               rng_;
    std::optional<const Request&> myRequest_{std::nullopt};
};

class GoodsDemander final {
  public:
    explicit GoodsDemander(RandomGenerator& masterRng) noexcept
        : trader_{masterRng}, mpc_{masterRng.random(setting::mpc)} {}

    [[nodiscard]] auto planAndRequestBudget(const Budget asset) noexcept -> Budget {
        const auto requestBudget = asset * mpc_;
        budget_                  = static_cast<Budget>(requestBudget);
        return *budget_;
    }

    void revisePlan(const Budget budget) noexcept {
        ASSERT(budget_);
        ASSERT(budget_ <= budget);
        budget_ = budget;
    }

    template <TryWithdrawFn F>
    void request(
        const AgentID id,
        F&&           withdrawFn,
        Market&       market,
        const int     sampleCnt = setting::goodsSampleCnt
    ) noexcept {
        ASSERT(budget_);
        const auto budget = *budget_;
        if (budget.isZeroOrLess()) return;
        trader_.request(id, budget, std::forward<F>(withdrawFn), market, sampleCnt);
    }

    void afterTrade(HHoldFinance& finance) noexcept { trader_.afterTrade(finance); }

  private:
    Trader                trader_;
    const double          mpc_;
    std::optional<Budget> budget_;
};
}  // namespace abm::goods::demander

namespace abm {
using GoodsDemander = goods::demander::GoodsDemander;
}