#pragma once

#include <utility>

#include "components/common.hpp"
#include "components/finance/deposit_demander.hpp"
#include "components/finance/deposit_supplier.hpp"
#include "core/setting.hpp"
#include "core/util.hpp"
#include "values/common.hpp"
#include "values/integrate.hpp"
#include "world/common.hpp"

namespace abm::finance {

class Bank {
  public:
    [[nodiscard]] explicit constexpr Bank() noexcept;

  private:
    DepositDemander depositDemander_;
};

class BankRegistry {
  public:
    [[nodiscard]] explicit constexpr BankRegistry() noexcept;

  private:
    std::vector<Bank> banks_;
};

class Finance {
  public:
    [[nodiscard]] explicit constexpr Finance(const Money asset, RandomGenerator& masterRng) noexcept
        : cash_{asset}, cashRatio_{masterRng.random(setting::cashRatio)} {}

    void assetPlus(const Money add) noexcept {
        ASSERT(add >= Money{0.0});

        // 現金比率が目標以上で、預金に成功した場合早期リターン
        if (currentCashRatio() > cashRatio_ and depositSupplier_.tryDeposit(add)) return;
        cash_ += add;
    }

    [[nodiscard]] auto claimBudget(const Money claim) noexcept -> Money {
        ASSERT(claim >= Money{0.0});
        ASSERT(asset() >= Money{0.0});

        if (currentCashRatio() <= cashRatio_) {
            const auto out = min(cash_, claim);
            cash_ -= out;
            return out;
        }

        const auto withdraw = depositSupplier_.tryWithdraw(claim);
        ASSERT(withdraw <= claim);

        const auto rest = claim - withdraw;
        ASSERT(cash_ >= Money{0.0});
        const auto cashOut = min(cash_, rest);
        cash_ -= cashOut;

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
    DepositSupplier depositSupplier_;
    Money           cash_;
    const double    cashRatio_;
};

class FirmFinance final {
  public:
    [[nodiscard]] explicit constexpr FirmFinance(RandomGenerator& masterRng) noexcept
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
    [[nodiscard]] explicit constexpr HHoldFinance(RandomGenerator& masterRng) noexcept
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
    [[nodiscard]] explicit constexpr GovernmentFinance() noexcept : asset_{0.0} {}

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