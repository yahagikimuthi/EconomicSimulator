#pragma once

#include "components/labor_supplier.hpp"
#include "core/forward.hpp"

namespace labor_supplier {
struct JobEntryView {
    JobEntryView(Component& comp) : comp_{comp} {}

    void isPosting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }

    void entry(const tbb::concurrent_vector<world::LaborEntry>::iterator it) {  // NOLINT
        comp_.posting_.myEntries_.emplace_back(it);
    }

    [[nodiscard]] auto productPower() const -> double { return comp_.parameter_.productPower_; }

  private:
    Component& comp_;
};
void jobEntry(
    JobEntryView view, const int id, tbb::concurrent_vector<world::LaborRequest>& requestBox
);
}  // namespace labor_supplier