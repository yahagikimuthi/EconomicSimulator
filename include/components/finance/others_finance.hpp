#pragma once

#include <algorithm>
#include <atomic>
#include <cassert>

#include "components/common.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "world/deposit.hpp"

namespace abm::finance {
class HHoldFinance final {
  public:
    explicit HHoldFinance(const AgentID id, RandomGenerator& masterRng) noexcept
        : bankAccount_{id},
          cash_{masterRng.random(setting::hholdInitialAsset)},
          cashRatio_{masterRng.random(setting::cashRatio)} {}

    [[nodiscard]] auto makeWithdrawFn() noexcept -> TryWithdrawFn auto {
        return [&] [[nodiscard]] (const Budget withdraw) noexcept -> Money {
            return tryWithdraw(withdraw);
        };
    }

    [[nodiscard]] auto makeDepositFn() noexcept -> DepositFn auto {
        return [&](const Money depositAmount) noexcept -> void { deposit(depositAmount); };
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
        if (currentCashRatio() > cashRatio_) {
            bankAccount_.deposit(add);
            return;
        }
        cash_ += add;
    }

    [[nodiscard]] static auto claimBudget(const Budget claim) noexcept -> Budget {
        assert(claim.isZeroOrMore());
        return claim;
    }

    [[nodiscard]] auto asset() const noexcept -> Budget {
        return static_cast<Budget>(cash_) + bankAccount_.balance();
    }

  private:
    [[nodiscard]] auto currentCashRatio() const noexcept -> double {
        if (asset().isZero()) return 0.0;
        return static_cast<Budget>(cash_) / asset();
    }

    BankAccount  bankAccount_;
    Money        cash_;
    const double cashRatio_;
};

class GovernmentFinance final {
  public:
    explicit GovernmentFinance() noexcept = default;

    [[nodiscard]] auto tryWithdraw(const Budget tryingWithdraw) noexcept -> Money {
        assert(tryingWithdraw.isZeroOrMore());
        const auto sub = Money{tryingWithdraw.value()};
        const auto out = Money{std::min(sub.value(), cash_.load())};
        cash_.fetch_sub(out.value());  // TODO 処理系が対応する場合store_subに

        return sub;
    }

    void deposit(const Money add) noexcept {
        assert(add.isZeroOrMore());
        cash_.fetch_add(add.value());  // TODO 処理系が対応する場合store_addに
    }

    [[nodiscard]] auto asset() const noexcept -> Budget {
        return static_cast<Budget>(cash_.load());
    }

  private:
    std::atomic<double> cash_{0.0};
};
}  // namespace abm::finance

namespace abm {
using HHoldFinance      = finance::HHoldFinance;
using GovernmentFinance = finance::GovernmentFinance;
}  // namespace abm