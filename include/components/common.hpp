#pragma once

#include <pcg_random.hpp>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "world/message.hpp"

namespace abm {
class [[nodiscard]] AgentIndex {
  public:
    AgentIndex(const AgentID id) : id_{id} {}
    auto id() const -> AgentID POST(id : id >= AgentID{0}) { return id_; }

  private:
    const AgentID id_;
};

class [[nodiscard]] BaseFinance {
  public:
    void assetPlus(const Money plus) { asset_ += plus; }
    auto asset() const -> Money { return asset_; }

  protected:
    BaseFinance(const Money asset) : asset_{asset} {}
    Money asset_;
};

class [[nodiscard]] FirmFinance : public BaseFinance {
  public:
    FirmFinance(const Money asset) : BaseFinance::BaseFinance(asset) {}
    void endStep(CensusDropBox& dropBox) const { dropBox.firmAssets.emplace_back(asset_.value()); }
};

class [[nodiscard]] HHoldFinance : public BaseFinance {
  public:
    HHoldFinance(const Money asset) : BaseFinance::BaseFinance(asset) {}
    void endStep(CensusDropBox& dropBox) const { dropBox.hholdAssets.emplace_back(asset_.value()); }
};
}  // namespace abm