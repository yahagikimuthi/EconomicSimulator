#pragma once

#include <cstddef>
#include <vector>

#include "components/labor_demander.hpp"
#include "core/base.hpp"
#include "world/message.hpp"

namespace labor_demander {
struct LayOffsView final : BaseView<Component> {
    using BaseView<Component>::BaseView;

    auto rosterSize() const -> int {
        const std::size_t rosterSize{comp_.humanResources_.companyBoard_->roster_.size()};
        return static_cast<int>(rosterSize);
    }
    auto getRosterEntry(const int idx) -> world::RosterEntry& {
        return comp_.humanResources_.companyBoard_->roster_[static_cast<std::size_t>(idx)];
    }
    void addEmptyRoster(world::RosterEntry& emptyRoster) {
        comp_.humanResources_.emptyRosterPool_.emplace_back(&emptyRoster);
    }
    auto emptyRosterPool() -> std::vector<SafePtr<world::RosterEntry>>& {
        return comp_.humanResources_.emptyRosterPool_;
    }
};

void layoffs(LayOffsView view, const int layOffsCnt);
}  // namespace labor_demander