#pragma once

#include <pcg_random.hpp>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "world/message.hpp"

namespace agent_index {
class [[nodiscard]] Component {
  public:
    Component(const AgentID id) : id_{id} {}
    auto id() const -> AgentID POST(id : id >= AgentID{0}) { return id_; }

  private:
    const AgentID id_;
};
};  // namespace agent_index

namespace finance {
class [[nodiscard]] FinanceBaseComponent {
  public:
    FinanceBaseComponent(const Money asset) : asset_{asset} {}
    void assetPlus(const Money plus) { asset_ += plus; }
    auto asset() const -> Money { return asset_; }

  protected:
    Money asset_;
};
}  // namespace finance

namespace firm_finance {
class [[nodiscard]] Component : public finance::FinanceBaseComponent {
  public:
    using finance::FinanceBaseComponent::FinanceBaseComponent;
    void endStep(CensusDropBox& dropBox) const { dropBox.firmAssets.emplace_back(asset_.value()); }
};
}  // namespace firm_finance

namespace hhold_finance {
class [[nodiscard]] Component : public finance::FinanceBaseComponent {
  public:
    using finance::FinanceBaseComponent::FinanceBaseComponent;
    void endStep(CensusDropBox& dropBox) const { dropBox.hholdAssets.emplace_back(asset_.value()); }
};
}  // namespace hhold_finance