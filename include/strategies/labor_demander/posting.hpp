#pragma once

#include <tbb/concurrent_vector.h>

#include "components/labor_demander.hpp"

namespace labor_demander {
struct [[nodiscard]] PostJobView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    void plan(const double wage, const int employ, const int offer) {
        auto& plan = comp_.plan_;
        plan.wage_ = wage, plan.employ_ = employ, plan.offer_ = offer;
    }
    void myRequest(tbb::concurrent_vector<world::LaborRequest>::iterator it) {  // NOLINT
        comp_.posting_.myRequest_ = &*it;
    }
    void posting(const bool isPosting) { comp_.posting_.isPosting_ = isPosting; }

    void isRecruiting(const bool isRecruiting) { comp_.plan_.isRecruiting = isRecruiting; }
};

void postJob(
    const int                                    id,
    const int                                    desiredEmploy,
    tbb::concurrent_vector<world::LaborRequest>& requestBox,
    PostJobView                                  view
);
}  // namespace labor_demander