#pragma once

#include <pcg_random.hpp>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "world/common.hpp"

namespace abm {
class AgentIndex final {
  public:
    [[nodiscard]] explicit AgentIndex(const AgentID id) : id_{id} {}
    [[nodiscard]] auto id() const -> AgentID POST(id : id >= AgentID{0}) { return id_; }

  private:
    const AgentID id_;
};

class BaseFinance {
  public:
    void assetPlus(const Money plus) { asset_ += plus; }

    [[nodiscard]] auto asset() const -> Money { return asset_; }

  protected:
    [[nodiscard]] explicit BaseFinance(const Money asset) : asset_{asset} {}
    Money asset_;
};

class FirmFinance final : public BaseFinance {
  public:
    [[nodiscard]] explicit FirmFinance(const Money asset) : BaseFinance::BaseFinance(asset) {}
    void endStep(CensusDropBox& dropBox) const { dropBox.firmAssets.emplace_back(asset_.value()); }
};

class HHoldFinance final : public BaseFinance {
  public:
    [[nodiscard]] explicit HHoldFinance(const Money asset) : BaseFinance::BaseFinance(asset) {}
    void endStep(CensusDropBox& dropBox) const { dropBox.hholdAssets.emplace_back(asset_.value()); }
};
}  // namespace abm