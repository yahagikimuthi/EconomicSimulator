#pragma once

#include <algorithm>
#include <cassert>
#include <concepts>
#include <cstdint>
#include <utility>

#include "components/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "world/deposit.hpp"
#include "world/drop_box.hpp"

namespace abm::finance {
template <typename F>
concept SubsidyFn = requires(F f) {
    { f() } -> std::same_as<Money>;
};

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

            netIncomeBeforeTax_ -= withdraw + cashOut;
            return withdraw + cashOut;
        }
        const auto cashOut = std::min(cash_, sub);
        cash_ -= cashOut;
        const auto rest        = sub - cashOut;
        const auto withdraw    = bankAccount_.withdraw(rest);
        const auto moreCashOut = rest - withdraw;
        cash_ -= moreCashOut;

        netIncomeBeforeTax_ -= cashOut + withdraw + moreCashOut;
        return cashOut + withdraw + moreCashOut;
    }

    void deposit(const Money add) noexcept {
        assert(add.isZeroOrMore());

        if (currentCashRatio() > cashRatio_)
            bankAccount_.deposit(add);
        else
            cash_ += add;

        netIncomeBeforeTax_ += add;
    }

    void logging(FinanceDropBox& dropBox) noexcept {
        dropBox.firmAssets.add(asset());
        dropBox.netIncome.add(netIncomeBeforeTax_);
    }

    template <PayTaxFn F1, SubsidyFn F2>
    void finalizeAccounts(F1&& payCorporateTaxFn, F2&& subsidyFn) noexcept {
        if (netIncomeBeforeTax_.isZero()) return;
        if (netIncomeBeforeTax_.isPositive()) {
            const auto netIncome = std::forward<F1>(payCorporateTaxFn)(netIncomeBeforeTax_);
            const auto paid      = netIncomeBeforeTax_ - netIncome;
            nothing(tryWithdraw(static_cast<Budget>(paid)));
            return;
        }
        const auto subsidy = std::forward<F2>(subsidyFn)();
        deposit(subsidy);
    }

  private:
    [[nodiscard]] auto currentCashRatio() const noexcept -> double {
        if (asset().isZero()) return 0.0;
        return static_cast<Budget>(cash_) / asset();
    }

    BankAccount  bankAccount_;
    Money        cash_;
    Money        netIncomeBeforeTax_{0.0};
    const double cashRatio_;
};
}  // namespace abm::finance

namespace abm {
using FirmFinance = finance::FirmFinance;
}