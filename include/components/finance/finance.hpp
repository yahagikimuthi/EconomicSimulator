#pragma once

#include <utility>

#include "components/common.hpp"
#include "components/finance/deposit_supplier.hpp"
#include "core/setting.hpp"
#include "core/util.hpp"
#include "core/values/common.hpp"
#include "world/common.hpp"

namespace abm::finance {
class BaseFinance {
  public:
    void assetPlus(const Money plus) noexcept { asset_ += plus; }

    [[nodiscard]] auto asset() const noexcept -> Money { return asset_; }

  protected:
    [[nodiscard]] explicit constexpr BaseFinance(const Money asset) noexcept : asset_{asset} {}

    DepositSupplier depositSupplier_;
    Money           asset_;
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