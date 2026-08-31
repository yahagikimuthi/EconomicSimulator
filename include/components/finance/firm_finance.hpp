#pragma once

#include <algorithm>

#include "components/finance/deposit_supplier.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"

namespace abm::finance {
class FirmFinance final {
    struct PL final {
        void reset() noexcept { sales = capitalGoodsCost = depreciation = taxes = Money{0.0}; }

        Money sales{0.0};
        Money capitalGoodsCost{0.0};
        Money depreciation{0.0};
        Money taxes{0.0};
    };

  public:
    explicit FirmFinance(const AgentID id, RandomGenerator& masterRng) noexcept
        : depositSupplier_{id},
          cash_{Money{masterRng.random(setting::firmInitialAsset)}},
          cashRatio_{masterRng.random(setting::cashRatio)} {}

    enum class AccountItem : char { Sales, CapitalGoodsCost, Depreciation, Taxes };

    [[nodiscard]] auto tryWithdraw(const Budget tryingWithdraw, const AccountItem item) noexcept
        -> Money {
        ASSERT(tryingWithdraw.isZeroOrMore());
        ASSERT(item != AccountItem::Sales);

        const auto sub = Money{tryingWithdraw.value()};
        if (currentCashRatio() > cashRatio_) {
            const auto withdraw = depositSupplier_.tryWithdraw(sub);
            ASSERT(withdraw <= sub);
            const auto cashOut = sub - withdraw;
            cash_ -= cashOut;
            postToPlFromMinus(withdraw + cashOut, item);
            return withdraw + cashOut;
        }
        const auto cashOut = std::min(cash_, sub);
        cash_ -= cashOut;
        const auto rest        = sub - cashOut;
        const auto withdraw    = depositSupplier_.tryWithdraw(rest);
        const auto moreCashOut = rest - withdraw;
        cash_ -= moreCashOut;
        postToPlFromPlus(cashOut + withdraw + moreCashOut, item);
        return cashOut + withdraw + moreCashOut;
    }

    void assetPlus(const Money add, const AccountItem item) noexcept {
        ASSERT(add.isZeroOrMore());

        postToPlFromPlus(add, item);
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

    void postToPlFromPlus(const Money money, const AccountItem item) noexcept {
        ASSERT(money.isZeroOrMore());
        switch (item) {
            case AccountItem::Sales:
                pl_.sales += money;
                break;
            case AccountItem::CapitalGoodsCost:
                pl_.capitalGoodsCost -= money;
                break;
            case AccountItem::Depreciation:
                pl_.depreciation -= money;
                break;
            case AccountItem::Taxes:
                pl_.taxes -= money;
                break;
        }
    }

    void postToPlFromMinus(const Money money, const AccountItem item) noexcept {
        ASSERT(money.isZeroOrMore());
        switch (item) {
            case AccountItem::Sales:
                pl_.sales -= money;
                break;
            case AccountItem::CapitalGoodsCost:
                pl_.capitalGoodsCost += money;
                break;
            case AccountItem::Depreciation:
                pl_.depreciation += money;
                break;
            case AccountItem::Taxes:
                pl_.taxes += money;
                break;
        }
    }

    PL              pl_;
    DepositSupplier depositSupplier_;
    Money           cash_;
    const double    cashRatio_;
};
}  // namespace abm::finance

namespace abm {
using FirmFinance = finance::FirmFinance;
}