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
};
}  // namespace firm_finance

namespace hhold_finance {
struct Component {
    double asset_;
    Component();

    [[nodiscard]] auto asset() const -> double { return asset_; }
};
}  // namespace hhold_finance