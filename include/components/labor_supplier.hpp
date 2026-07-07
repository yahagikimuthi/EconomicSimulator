#pragma once

#include <tbb/concurrent_vector.h>
#include <functional>

#include "core/forward.hpp"

namespace labor_supplier {
struct Posting {
    using LaborMarketCoordinate = std::pair<
        std::reference_wrapper<const world::LaborRequest>,
        tbb::concurrent_vector<world::LaborEntry>::iterator>;

    std::vector<LaborMarketCoordinate> myEntries_;
    bool                               isPosting_{false};
};
struct Contraction {
    int    firmID_{-1};
    double wage_{};
};
struct Parameter {
    double productPower_;
};

struct Component {
    Posting     posting_;
    Contraction contraction_;
    Parameter   parameter_;

    Component();

    [[nodiscard]] auto getWage() const -> double { return contraction_.wage_; }
};
}  // namespace labor_supplier