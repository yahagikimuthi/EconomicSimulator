#pragma once

#include <algorithm>

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

    [[nodiscard]] auto withdraw(const Money sub) noexcept -> Money {
        ASSERT(sub.isZeroOrMore());
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

    [[nodiscard]] auto tryWithdraw(const Budget sub) noexcept -> Money {
        return withdraw(Money{sub.value()});
    }

    void assetPlus(const Money add) noexcept {
        ASSERT(add.isZeroOrMore());
        if (currentCashRatio() > cashRatio_) {
            depositSupplier_.deposit(add);
            return;
        }
        cash_ += add;
    }

    [[nodiscard]] auto claimBudget(const Budget claim) const noexcept -> Budget {
        ASSERT(claim.isZeroOrMore());

        const auto balance = static_cast<Budget>(depositSupplier_.balance());
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

    [[nodiscard]] auto asset() const noexcept -> Money {
        return cash_ + depositSupplier_.balance();
    }

  private:
    [[nodiscard]] auto currentCashRatio() const noexcept -> double {
        if (asset().isZero()) return 0.0;
        return cash_ / asset();
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