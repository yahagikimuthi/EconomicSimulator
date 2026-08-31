#pragma once

#include <algorithm>

#include "components/finance/deposit_supplier.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"

namespace abm::finance {
enum class AccountItem : char { Sales, CapitalGoodsCost, Depreciation, Taxes };

struct PL {
    void reset() noexcept { sales = capitalGoodsCost = depreciation = taxes = Money{0.0}; }

    Money sales{0.0};
    Money capitalGoodsCost{0.0};
    Money depreciation{0.0};
    Money taxes{0.0};
};

struct BS {};

enum class Color : char { Red, Blue, Yellow };

class FirmFinance {
  public:
    explicit FirmFinance(const Money asset, RandomGenerator& masterRng) noexcept
        : cash_{asset}, cashRatio_{masterRng.random(setting::cashRatio)} {}

    [[nodiscard]] auto withdraw(const Money sub, const AccountItem item) noexcept -> Money {
        ASSERT(sub.isZeroOrMore());
        ASSERT(item != AccountItem::Sales);
        if (currentCashRatio() > cashRatio_) {
            const auto withdraw = depositSupplier_.tryWithdraw(sub);
            ASSERT(withdraw <= sub);
            const auto cashOut = sub - withdraw;
            cash_ -= cashOut;
            postToPl(withdraw + cashOut, item);
            return withdraw + cashOut;
        }
        const auto cashOut = std::min(cash_, sub);
        cash_ -= cashOut;
        const auto rest        = sub - cashOut;
        const auto withdraw    = depositSupplier_.tryWithdraw(rest);
        const auto moreCashOut = rest - withdraw;
        cash_ -= moreCashOut;
        postToPl(cashOut + withdraw + moreCashOut, item);
        return cashOut + withdraw + moreCashOut;
    }

    void assetPlus(const Money add, const AccountItem item) noexcept {
        ASSERT(add.isZeroOrMore());
        ASSERT(item == AccountItem::Sales);

        pl_.sales += add;
        // 現金比率が目標以上で、預金に成功した場合早期リターン
        if (currentCashRatio() > cashRatio_ and depositSupplier_.tryDeposit(add)) return;
        cash_ += add;
    }

    [[nodiscard]] auto claimBudget(const Money claim) const noexcept -> Money {
        ASSERT(claim.isZeroOrMore());

        if (currentCashRatio() > cashRatio_) {
            const auto cashOut     = std::min(cash_, claim);
            const auto rest        = claim - cashOut;
            const auto withdraw    = std::min(static_cast<Money>(depositSupplier_.balance()), rest);
            const auto moreCashOut = rest - withdraw;
            return cashOut + withdraw + moreCashOut;
        }

        const auto withdraw = std::min(static_cast<Money>(depositSupplier_.balance()), claim);
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

    void postToPl(const Money money, const AccountItem item) noexcept {
        ASSERT(money.isZeroOrMore());
        switch (item) {
            case AccountItem::Sales:
                pl_.sales += money;
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
            default:
        }
    }

    PL              pl_;
    DepositSupplier depositSupplier_;
    Money           cash_;
    const double    cashRatio_;
};

class HHoldFinance final {
  public:
    explicit HHoldFinance(const Money asset, RandomGenerator& masterRng) noexcept
        : cash_{asset}, cashRatio_{masterRng.random(setting::cashRatio)} {}

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

    void assetPlus(const Money add) noexcept {
        ASSERT(add.isZeroOrMore());
        // 現金比率が目標以上で、預金に成功した場合早期リターン
        if (currentCashRatio() > cashRatio_ and depositSupplier_.tryDeposit(add)) return;
        cash_ += add;
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
using FirmFinance       = finance::FirmFinance;
using GovernmentFinance = finance::GovernmentFinance;
}  // namespace abm