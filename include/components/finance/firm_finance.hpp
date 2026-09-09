#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>

#include "components/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "world/deposit.hpp"

namespace abm::finance {
class FirmFinance final {
  public:
    explicit FirmFinance(const AgentID id, RandomGenerator& masterRng) noexcept
        : bankAccount_{id},
          cash_{masterRng.random(setting::firmInitialAsset)},
          cashRatio_{masterRng.random(setting::cashRatio)} {}

    enum class AccountItem : std::uint8_t { Sales, PersonalCost, CapitalGoodsCost };

    [[nodiscard]] auto makeWithdrawFn() noexcept -> TryWithdrawFn auto {
        return [&] [[nodiscard]] (const Budget withdraw) noexcept -> Money {
            return tryWithdraw(withdraw);
        };
    }

    [[nodiscard]] auto makeDepositFn() noexcept -> DepositFn auto {
        return [&](const Money depositAmount) noexcept -> void { deposit(depositAmount); };
    }

    [[nodiscard]] static auto claimBudget(const Budget claim) noexcept -> Budget {
        assert(claim.isZeroOrMore());
        return claim;
    }

    [[nodiscard]] auto asset() const noexcept -> Budget {
        return static_cast<Budget>(cash_) + bankAccount_.balance();
    }

    [[nodiscard]] auto tryWithdraw(const Budget tryingWithdraw) noexcept -> Money {
        assert(tryingWithdraw.isZeroOrMore());

        const auto sub = Money{tryingWithdraw.value()};
        if (currentCashRatio() > cashRatio_) {
            const auto withdraw = bankAccount_.withdraw(sub);
            assert(withdraw <= sub);
            const auto cashOut = sub - withdraw;
            cash_ -= cashOut;
            return withdraw + cashOut;
        }
        const auto cashOut = std::min(cash_, sub);
        cash_ -= cashOut;
        const auto rest        = sub - cashOut;
        const auto withdraw    = bankAccount_.withdraw(rest);
        const auto moreCashOut = rest - withdraw;
        cash_ -= moreCashOut;
        return cashOut + withdraw + moreCashOut;
    }

    void deposit(const Money add) noexcept {
        assert(add.isZeroOrMore());

        if (currentCashRatio() > cashRatio_)
            bankAccount_.deposit(add);
        else
            cash_ += add;
    }

  private:
    [[nodiscard]] auto currentCashRatio() const noexcept -> double {
        if (asset().isZero()) return 0.0;
        return static_cast<Budget>(cash_) / asset();
    }

    BankAccount  bankAccount_;
    Money        cash_;
    Money        netIncome_{0.0};
    const double cashRatio_;
};
}  // namespace abm::finance

namespace abm {
using FirmFinance = finance::FirmFinance;
}