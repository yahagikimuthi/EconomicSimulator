#pragma once

#include <cstdint>
#include <pcg_random.hpp>
#include <world/message.hpp>

#include "core/base.hpp"

namespace agent_index {
class [[nodiscard]] Component {
  public:
    Component(const int id) : id_{id} {}
    auto id() const -> int POST(id : id >= 0) { return id_; }

  private:
    const int id_;
};
};  // namespace agent_index

namespace firm_finance {
class [[nodiscard]] Component {
  public:
    Component(pcg32& masterRng);
    void assetPlus(const double plus) { asset_ += plus; }
    void endStep(world::CensusDropBox& dropBox) const { dropBox.firmAssets.emplace_back(asset_); }

  private:
    double asset_;
};
}  // namespace firm_finance

namespace hhold_finance {
class [[nodiscard]] Component {
  public:
    Component(pcg32& masterRng);
    auto asset() const -> double { return asset_; }
    void assetPlus(const double plus) { asset_ += plus; }
    void endStep(world::CensusDropBox& dropBox) const { dropBox.hholdAssets.emplace_back(asset_); }

  private:
    double asset_;
};
}  // namespace hhold_finance