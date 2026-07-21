#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <pcg_random.hpp>

#include "core/base.hpp"
#include "world/message.hpp"

namespace labor_demander {
struct Log {
    double wage_;
    int    actualEmploy_;
    int    offerPlan_;
    int    applicantNum_;
};
struct Plan {
    bool   isRecruiting{false};
    double wage_{};
    int    employ_{};
    int    offer_{};
};
struct HR {
    const std::size_t        myCompanyBoardIdx_;
    std::vector<std::size_t> emptyRosterIdxPool_;
    double                   sumWage_;
    int                      employeeCnt;
};
struct EmploymentLedger {
    int    applicantNum_;
    int    employing_;
    double sumWage_;
};
struct Parameter {
    double       offerRate_;
    const double wageAdjustmentVolatility_;
    const double employAdjustmentVolatility_;
    const double offerAdjustmentVolatility_;
};
struct Posting {
    SafePtr<world::LaborRequest>                  myRequest_{nullptr};
    std::vector<SafePtr<const world::LaborEntry>> offerApplicants_;
    bool                                          isPosting_;
};
struct Component {
    pcg32            rng_;
    Log              log_;
    Plan             plan_;
    Posting          posting_{};
    HR               humanResources_;
    EmploymentLedger employmentLedger{};
    Parameter        parameter_;

    Component(
        const std::uint64_t state, const std::uint64_t stream, const std::size_t myCompanyBoard
    );

    [[nodiscard]] auto sumWage() const -> double { return humanResources_.sumWage_; }
    [[nodiscard]] auto employeeCnt(const std::span<const world::CompanyBoard>& companyBoards) const
        -> int {
        const auto&       myRoster = companyBoards[humanResources_.myCompanyBoardIdx_].roster_;
        const std::size_t rosterSize{myRoster.size() - humanResources_.emptyRosterIdxPool_.size()};
        return static_cast<int>(rosterSize);
    }
};
}  // namespace labor_demander