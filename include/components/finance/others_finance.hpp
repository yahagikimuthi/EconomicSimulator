#pragma once

#include <algorithm>

#include "components/common.hpp"
#include "components/finance/deposit_supplier.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"

namespace abm::finance {
class HHoldFinance final {
  public:
    explicit HHoldFinance(const AgentID id, RandomGenerator& masterRng) noexcept
        : depositSupplier_{id},
          cash_{Money{masterRng.random(setting::hholdInitialAsset)}},
          cashRatio_{masterRng.random(setting::cashRatio)} {}

    [[nodiscard]] auto makeWithdrawFn() noexcept -> TryWithdrawFn auto {
        return [&](const Budget withdraw) noexcept -> Money { return tryWithdraw(withdraw); };
    }

    [[nodiscard]] auto makeDepositFn() noexcept -> DepositFn auto {
        return [&](const Money depositAmount) noexcept -> void { deposit(depositAmount); };
    }

    [[nodiscard]] auto tryWithdraw(const Budget tryingWithdraw) noexcept -> Money {
        ASSERT(tryingWithdraw.isZeroOrMore());
        const auto sub = Money{tryingWithdraw.value()};
        if (currentCashRatio() > cashRatio_) {
            const auto withdraw = depositSupplier_.tryWithdraw(sub);
            ASSERT(withdraw <= sub);
            const auto cashOut = sub - withdraw;
            cash_ -= cashOut;
            return withdraw + cashOut;
        }
        const auto cashOut = std::min(cash_, sub);
        cash_ -= cashOut;
        const auto rest        = sub - cashOut;
        const auto withdraw    = depositSupplier_.tryWithdraw(rest);
        const auto moreCashOut = rest - withdraw;
        cash_ -= moreCashOut;
        return cashOut + withdraw + moreCashOut;
    }

    void deposit(const Money add) noexcept {
        ASSERT(add.isZeroOrMore());
        if (currentCashRatio() > cashRatio_) {
            depositSupplier_.deposit(add);
            return;
        }
        cash_ += add;
    }

    [[nodiscard]] auto claimBudget(const Budget claim) const noexcept -> Budget {
        ASSERT(claim.isZeroOrMore());

        const auto balance = depositSupplier_.balance();
        if (currentCashRatio() > cashRatio_) {
            const auto cashOut     = std::min(static_cast<Budget>(cash_), claim);
            const auto rest        = claim - cashOut;
            const auto withdraw    = std::min(balance, rest);
            const auto moreCashOut = rest - withdraw;
            return cashOut + withdraw + moreCashOut;
        }

        const auto withdraw = std::min(balance, claim);
        ASSERT(withdraw <= claim);
        const auto cashOut = claim - withdraw;
        return withdraw + cashOut;
    }

    [[nodiscard]] auto asset() const noexcept -> Budget {
        return static_cast<Budget>(cash_) + depositSupplier_.balance();
    }

  private:
    [[nodiscard]] auto currentCashRatio() const noexcept -> double {
        if (asset().isZero()) return 0.0;
        return static_cast<Budget>(cash_) / asset();
    }

    DepositSupplier depositSupplier_;
    Money           cash_;
    const double    cashRatio_;
};

class GovernmentFinance final {
  public:
    explicit GovernmentFinance() noexcept : asset_{0.0} {}

    void assetPlus(const Money plus) noexcept { asset_ += plus; }

    [[nodiscard]] auto asset() const noexcept -> Money { return asset_; }

  private:
    Money asset_;
};
}  // namespace abm::finance

namespace abm {
using HHoldFinance      = finance::HHoldFinance;
using GovernmentFinance = finance::GovernmentFinance;
}  // namespace abm