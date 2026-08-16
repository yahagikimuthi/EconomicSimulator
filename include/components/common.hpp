#pragma once

#include <pcg_random.hpp>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "world/message.hpp"

namespace abm {
class AgentIndex {
  public:
    [[nodiscard]] AgentIndex(const AgentID id) : id_{id} {}
    [[nodiscard]] auto id() const -> AgentID POST(id : id >= AgentID{0}) { return id_; }

  private:
    const AgentID id_;
};

class BaseFinance {
  public:
    void               assetPlus(const Money plus) { asset_ += plus; }
    [[nodiscard]] auto asset() const -> Money { return asset_; }

  protected:
    [[nodiscard]] BaseFinance(const Money asset) : asset_{asset} {}
    Money asset_;
};

class FirmFinance : public BaseFinance {
  public:
    [[nodiscard]] FirmFinance(const Money asset) : BaseFinance::BaseFinance(asset) {}
    void endStep(CensusDropBox& dropBox) const { dropBox.firmAssets.emplace_back(asset_.value()); }
};

class HHoldFinance : public BaseFinance {
  public:
    [[nodiscard]] HHoldFinance(const Money asset) : BaseFinance::BaseFinance(asset) {}
    void endStep(CensusDropBox& dropBox) const { dropBox.hholdAssets.emplace_back(asset_.value()); }
};
}  // namespace abm