#pragma once

#include <pcg_random.hpp>
#include <world/message.hpp>

#include "core/base.hpp"
#include "core/values/common.hpp"

namespace agent_index {
class [[nodiscard]] Component {
  public:
    Component(const AgentID id) : id_{id} {}
    auto id() const -> AgentID POST(id : id >= AgentID{0}) { return id_; }

  private:
    const AgentID id_;
};
};  // namespace agent_index

namespace firm_finance {
class [[nodiscard]] Component {
  public:
    Component(pcg32& masterRng);
    void assetPlus(const Money plus) { asset_ += plus; }
    auto asset() const -> Money { return asset_; }
    void endStep(world::CensusDropBox& dropBox) const {
        dropBox.firmAssets.emplace_back(asset_.value());
    }

  private:
    Money asset_;
};
}  // namespace firm_finance

namespace hhold_finance {
class [[nodiscard]] Component {
  public:
    Component(pcg32& masterRng);
    auto asset() const -> Money { return asset_; }
    void assetPlus(const Money plus) { asset_ += plus; }
    void endStep(world::CensusDropBox& dropBox) const {
        dropBox.hholdAssets.emplace_back(asset_.value());
    }

  private:
    Money asset_;
};
}  // namespace hhold_finance