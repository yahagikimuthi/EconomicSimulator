#pragma once

namespace agent_index {
struct Component {
    const int id_;
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
};
}  // namespace hhold_finance