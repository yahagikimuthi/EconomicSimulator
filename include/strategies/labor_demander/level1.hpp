#pragma once

#include <tbb/concurrent_vector.h>

#include "components/labor_demander.hpp"

namespace labor_demander {
struct PostJobCtx {
    PostJobCtx(Component& comp) : comp_{comp} {}

    void setPlan(const double wage, const int employ) {
        comp_.plan_.wage_ = wage, comp_.plan_.employ_ = employ;
    }
    void setMyRequest(const tbb::concurrent_vector<world::LaborRequest>::iterator it) {  // NOLINT
        comp_.posting_.myRequest_ = it;
    }

  private:
    Component& comp_;
};

void postJob(const int id, PostJobCtx ctx, const Component& comp);
}  // namespace labor_demander