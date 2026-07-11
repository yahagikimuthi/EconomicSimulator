#pragma once

namespace agent_index {
struct Component {
    const int id_;

    Component(const int id) : id_{id} {}

    [[nodiscard]] auto id() const -> int { return id_; }
};
};  // namespace agent_index

namespace firm_finance {
struct Component {
    double asset_;
    Component();

    void assetPlus(const double plus) { asset_ += plus; }
};
}  // namespace firm_finance

namespace hhold_finance {
struct Component {
    double asset_;
    Component();

    [[nodiscard]] auto asset() const -> double { return asset_; }
    void               assetPlus(const double plus) { asset_ += plus; }
};
}  // namespace hhold_finance