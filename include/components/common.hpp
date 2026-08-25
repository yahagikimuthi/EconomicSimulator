#pragma once

#include <pcg_random.hpp>

#include "core/setting.hpp"
#include "core/util.hpp"
#include "core/values/common.hpp"
#include "world/common.hpp"

namespace abm {
class BaseFinance {
  public:
    void assetPlus(const Money plus) noexcept { asset_ += plus; }

    [[nodiscard]] auto asset() const noexcept -> Money { return asset_; }

  protected:
    [[nodiscard]] explicit constexpr BaseFinance(const Money asset) noexcept : asset_{asset} {}
    Money asset_;
};

class FirmFinance final {
  public:
    [[nodiscard]] explicit constexpr FirmFinance(RandomGenerator& masterRng) noexcept
        : asset_{masterRng.random(setting::agent_finance::firm)} {}

    void endStep(CensusDropBox& dropBox) noexcept {
        dropBox.firmAssets.emplace_back(asset_.value());
        reset();
    }

    void assetPlus(const Money plus) noexcept {
        asset_ += plus;
        thisPeriodProfit_ += plus;
    }

    [[nodiscard]] auto asset() const noexcept -> Money { return asset_; }

    void reset() noexcept { thisPeriodProfit_ = Money{0.0}; }

  private:
    Money asset_;
    Money thisPeriodProfit_{0.0};
};

class HHoldFinance final : public BaseFinance {
  public:
    [[nodiscard]] explicit constexpr HHoldFinance(RandomGenerator& masterRng) noexcept
        : BaseFinance::BaseFinance(Money{masterRng.random(setting::agent_finance::hhold)}) {}
    void endStep(CensusDropBox& dropBox) const noexcept {
        dropBox.hholdAssets.emplace_back(asset_.value());
    }
};

class GovernmentFinance final : public BaseFinance {
  public:
    [[nodiscard]] explicit constexpr GovernmentFinance() noexcept
        : BaseFinance::BaseFinance(Money{0.0}) {}
};
}  // namespace abm