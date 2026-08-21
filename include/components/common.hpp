#pragma once

#include <pcg_random.hpp>

#include "core/values/common.hpp"
#include "setting.hpp"
#include "util.hpp"
#include "world/common.hpp"

namespace abm {
class AgentIndex final {
  public:
    [[nodiscard]] explicit constexpr AgentIndex(const AgentID id) noexcept : id_{id} {}
    [[nodiscard]] auto id() const noexcept -> AgentID { return id_; }

  private:
    const AgentID id_;
};

class BaseFinance {
  public:
    void assetPlus(const Money plus) noexcept { asset_ += plus; }

    [[nodiscard]] auto asset() const noexcept -> Money { return asset_; }

  protected:
    [[nodiscard]] explicit constexpr BaseFinance(const Money asset) noexcept : asset_{asset} {}
    Money asset_;
};

class FirmFinance final : public BaseFinance {
  public:
    [[nodiscard]] explicit constexpr FirmFinance(RandomGenerator& masterRng) noexcept
        : BaseFinance::BaseFinance(Money{masterRng.random(setting::agent_finance::firm)}) {}
    void endStep(CensusDropBox& dropBox) const noexcept {
        dropBox.firmAssets.emplace_back(asset_.value());
    }
};

class HHoldFinance final : public BaseFinance {
  public:
    [[nodiscard]] explicit constexpr HHoldFinance(RandomGenerator& masterRng) noexcept
        : BaseFinance::BaseFinance(Money{masterRng.random(setting::agent_finance::hhold)}) {}
    void endStep(CensusDropBox& dropBox) const noexcept {
        dropBox.hholdAssets.emplace_back(asset_.value());
    }
};
}  // namespace abm