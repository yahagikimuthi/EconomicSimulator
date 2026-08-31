#pragma once

#include <algorithm>
#include <utility>

#include "components/common.hpp"
#include "components/finance/deposit_demander.hpp"
#include "components/finance/deposit_supplier.hpp"
#include "others/setting.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "world/common.hpp"

namespace abm::finance {

class Bank final {
  public:
    explicit Bank() noexcept;

  private:
    DepositDemander depositDemander_;
};

class BankRegistry final {
  public:
    explicit BankRegistry() noexcept;

  private:
    std::vector<Bank> banks_;
};

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

class Finance {
  public:
    explicit Finance(const Money asset, RandomGenerator& masterRng) noexcept
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
        ASSERT(add >= Money{0.0});
        ASSERT(item == AccountItem::Sales);

        pl_.sales += add;
        // 現金比率が目標以上で、預金に成功した場合早期リターン
        if (currentCashRatio() > cashRatio_ and depositSupplier_.tryDeposit(add)) return;
        cash_ += add;
    }

    [[nodiscard]] auto claimBudget(const Money claim) const noexcept -> Money {
        ASSERT(claim >= Money{0.0});
        ASSERT(asset() >= Money{0.0});

        if (currentCashRatio() > cashRatio_) {
            const auto cashOut  = std::min(cash_, claim);
            const auto rest     = claim - cashOut;
            const auto withdraw = std::min(static_cast<Money>(depositSupplier_.balance()), rest);
            return cashOut + withdraw;
        }

        const auto withdraw = std::min(static_cast<Money>(depositSupplier_.balance()), claim);
        ASSERT(withdraw <= claim);

        const auto rest = claim - withdraw;
        ASSERT(cash_ >= Money{0.0});
        const auto cashOut = std::min(cash_, rest);
        return withdraw + cashOut;
    }

    [[nodiscard]] auto asset() const noexcept -> Money {
        return cash_ + depositSupplier_.balance();
    }

    [[nodiscard]] auto currentCashRatio() const noexcept -> double {
        ASSERT(cash_ >= Money{0.0});
        ASSERT(asset() >= Money{0.0});
        if (asset() == Money{0.0}) return 0.0;
        return cash_ / asset();
    }

  private:
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

class FirmFinance final {
  public:
    explicit FirmFinance(RandomGenerator& masterRng) noexcept
        : asset_{masterRng.random(setting::firmInitialAsset)} {}

    template <AfterTaxCalculatorFn F>
    void endStep(F&& afterTaxCalculator, CensusDropBox& dropBox) noexcept {
        const auto afterTax = std::forward<F>(afterTaxCalculator)(thisPeriodProfit_);
        asset_ += afterTax;
        dropBox.firmAssets.emplace_back(asset_.value());
        thisPeriodProfit_ = Money{0.0};
    }

    void assetPlus(const Money plus) noexcept { thisPeriodProfit_ += plus; }

    [[nodiscard]] auto asset() const noexcept -> Money { return asset_ + thisPeriodProfit_; }

  private:
    Money asset_;
    Money thisPeriodProfit_{0.0};
};

class HHoldFinance final {
  public:
    explicit HHoldFinance(RandomGenerator& masterRng) noexcept
        : asset_{masterRng.random(setting::hholdInitialAsset)} {}

    void endStep(CensusDropBox& dropBox) const noexcept {
        dropBox.hholdAssets.emplace_back(asset_.value());
    }

    void assetPlus(const Money plus) noexcept { asset_ += plus; }

    [[nodiscard]] auto asset() const noexcept -> Money { return asset_; }

  private:
    Money asset_;
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
using FirmFinance       = finance::FirmFinance;
using HHoldFinance      = finance::HHoldFinance;
using GovernmentFinance = finance::GovernmentFinance;
}  // namespace abm