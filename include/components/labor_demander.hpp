#pragma once

#include <tbb/concurrent_vector.h>

#include "core/forward.hpp"

namespace labor_demander {
struct Log {
    double wage_;
    int    targetEmploy_;
    int    actualEmploy_;
};
struct Plan {
    double wage_;
    int    employ_;
};
struct HR {
    double sumWage_;
    int    employeeCnt;
};
struct Parameter {
    const double wageAdjustmentVolatility_;
    const double employAdjustmentVolatility_;
};
struct Posting {
    tbb::concurrent_vector<world::LaborRequest>::iterator  myRequest_;
    std::vector<std::reference_wrapper<world::LaborEntry>> offerApplicants_;
    bool                                                   isPosting_;
};
struct Component {
    Log       log_;
    Plan      plan_{};
    Posting   posting_{};
    HR        humanResources{};
    Parameter parameter_;

    Component();

    [[nodiscard]] auto getSumWage() const -> double { return humanResources.sumWage_; }
};
}  // namespace labor_demander