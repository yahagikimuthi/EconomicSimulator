#pragma once

#include "components/labor_demander/concepts.hpp"
#include "components/labor_demander/hr_manager.hpp"
#include "components/labor_demander/recruiter.hpp"

namespace labor::demander {
class [[nodiscard]] HumanResourceManagerTester {
  public:
    HumanResourceManagerTester(HumanResourceManager& manager) : manager_{manager} {}
    auto emptyRosterPool() -> auto& { return manager_.emptyRosterPool_; }
    auto roster() -> auto& { return manager_.companyBoard_.roster; }

  private:
    HumanResourceManager& manager_;
};

template <IPlanner T>
class [[nodiscard]] RecruiterTester {
  public:
    RecruiterTester(Recruiter<T>& recruiter) : recruiter_{recruiter} {}
    auto isRecruiting() -> bool& { return recruiter_.isRecruiting_; }
    auto isPosting() -> bool& { return recruiter_.isPosting_; }
    auto myRequest() -> SafePtr<world::LaborRequest>& { return recruiter_.myRequest_; }
    auto remainOfferNum() -> HeadCount& { return recruiter_.ledger_.remainOfferNum; }
    auto applicantNum() -> HeadCount& { return recruiter_.ledger_.applicantNum; }
    auto offerApplicants() -> auto& { return recruiter_.offerApplicants_; }
    auto employing() -> HeadCount& { return recruiter_.ledger_.employing; }

  private:
    Recruiter<T>& recruiter_;
};
}  // namespace labor::demander