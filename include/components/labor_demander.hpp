#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <pcg_random.hpp>

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
struct EmploymentLedger {
    int    employing_;
    double sumWage_;
};
struct Parameter {
    const double wageAdjustmentVolatility_;
    const double employAdjustmentVolatility_;
    const double fillRateThreshold_;
};
struct Posting {
    world::LaborRequest*                                   myRequest_;
    std::vector<std::reference_wrapper<world::LaborEntry>> offerApplicants_;
    bool                                                   isPosting_;
};
struct Component {
    pcg32            rng_;
    Log              log_;
    Plan             plan_{};
    Posting          posting_{};
    HR               humanResources{};
    EmploymentLedger employmentLedger{};
    Parameter        parameter_;

    Component(const std::uint64_t state, const std::uint64_t stream);

    [[nodiscard]] auto sumWage() const -> double { return employmentLedger.sumWage_; }
    [[nodiscard]] auto employeeCnt() const -> int { return employmentLedger.employing_; }
};
}  // namespace labor_demander