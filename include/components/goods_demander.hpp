#pragma once

#include <optional>
#include <utility>

#include "components/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "world/base_goods.hpp"

namespace abm::goods::demander {
class Trader final {
  public:
    explicit Trader(RandomGenerator& masterRng)
        : rng_{{masterRng.makeUint64(), masterRng.makeUint64()}} {}

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

    template <DepositFn F>
    void afterTrade(F&& depositFn) noexcept {
        if (not myRequest_) return;
        const auto remain = myRequest_->remainPaid();
        std::forward<F>(depositFn)(remain);
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

    [[nodiscard]] auto requestBudget(const Budget asset) noexcept -> Budget {
        const auto requestBudget = asset * mpc_;
        budget_                  = static_cast<Budget>(requestBudget);
        return *budget_;
    }

    void revisePlan(const Budget budget) noexcept {
        ASSERT(budget_);
        ASSERT(budget <= budget_);
        budget_ = budget;
    }

    template <TryWithdrawFn F>
    void request(const AgentID id, F&& withdrawFn, Market& market) noexcept {
        ASSERT(budget_);
        if (budget_->isZeroOrLess()) return;
        const auto availableCash = std::forward<F>(withdrawFn)(*budget_);
        trader_.request(id, availableCash, market);
    }

    template <DepositFn F>
    void afterTrade(F&& depositFn) noexcept {
        trader_.afterTrade(std::forward<F>(depositFn));
        reset();
    }

  private:
    void reset() noexcept {
        trader_.reset();
        budget_.reset();
    }

    Trader                trader_;
    const double          mpc_;
    std::optional<Budget> budget_;
};
}  // namespace abm::goods::demander

namespace abm {
using GoodsDemander = goods::demander::GoodsDemander;
}