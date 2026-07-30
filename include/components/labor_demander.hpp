#pragma once

#include <tbb/concurrent_vector.h>
#include <concepts>
#include <numeric>
#include <pcg_random.hpp>
#include <ranges>

#include "core/base.hpp"
#include "world/message.hpp"

namespace labor_demander {
class [[nodiscard]] RequestPlanner {
  public:
    RequestPlanner(pcg32& masterRng);

    void judgePlan(const int desiredEmploy);
    auto wagePlan() const -> double POST(wage : wage > 0.0) { return plan_.wage; }
    auto offerPlan() const -> int POST(employ : employ >= 0) { return plan_.offer; }
    void endStep(world::CensusDropBox& dropBox, const int actualEmploy, const int applicantNum)
        PRE(actualEmploy >= 0) PRE(applicantNum >= 0);

  private:
    auto calcNextWage() const -> double POST(wage : wage > 0.0);
    auto calcNextOffer(const int desiredEmploy) const -> int POST(offer : offer >= 0);
    auto updateOfferRate(const int actualEmploy) const -> double POST(rate : rate > 0.0);

    mutable pcg32 rng_;
    struct {
        double wage;
        int    employ;
        int    offer;
        void   reset() { wage = 0.0, employ = 0, offer = 0; }
    } plan_{};

    struct {
        double wage;
        int    actualEmploy;
        int    offerPlan;
        int    applicantNum;
    } log_{};

    struct {
        double       offerRate;
        const double wageAdjustVol;
        const double offerAdjustVol;
    } param_;
};

template <typename T>
concept HasAddRoster =
    requires(T& t, const int id, const double wage, world::Workspace& workspace) {
        { t.addRoster(id, wage, workspace) } -> std::same_as<SafePtr<world::RosterEntry>>;
    };
class [[nodiscard]] Recruiter {
  public:
    Recruiter(pcg32& masterRng);

    void post(
        const int                                    id,
        const int                                    desiredEmploy,
        tbb::concurrent_vector<world::LaborRequest>& requestBox
    ) PRE(id >= 0) PRE(desiredEmploy >= 0);
    void offer();
    void endStep(world::CensusDropBox& dropBox);
    auto isPosting() const -> bool { return isPosting_; }
    auto offerApplicants() -> std::vector<SafePtr<world::LaborEntry>>& { return offerApplicants_; }
    auto myRequest() -> const world::LaborRequest& { return *myRequest_; }
    void addEmployingLedger(const int plus) { ledger_.employing += plus; }

  private:
    RequestPlanner planner_;

    SafePtr<world::LaborRequest>            myRequest_{nullptr};
    std::vector<SafePtr<world::LaborEntry>> offerApplicants_;
    bool                                    isPosting_{false};

    struct {
        int  remainOfferNum;
        int  applicantNum;
        int  employing;
        void reset() { remainOfferNum = 0, applicantNum = 0, employing = 0; }
    } ledger_{};

    bool isRecruiting_{false};
};

class [[nodiscard]] HumanResourceManager {
  public:
    HumanResourceManager(world::CompanyBoard& companyBoard);
    auto addRoster(const int id, const double wage, world::Workspace& workspace)
        -> SafePtr<world::RosterEntry> PRE(id >= 0) PRE(wage > 0.0);
    void acceptResignation();
    void layOffs(const int layOffsCnt) PRE(layOffsCnt > 0);
    auto employeeCnt() const -> int POST(cnt : cnt >= 0) {
        const std::size_t rosterSize{companyBoard_.roster.size() - emptyRosterPool_.size()};
        return static_cast<int>(rosterSize);
    }
    auto sumWage() const -> double POST(wage : wage >= 0.0) {
        auto& roster = companyBoard_.roster;
        auto  view{
            roster | std::views::filter([](const world::RosterEntry& e) -> bool {
                return not e.isOccupied;
            }) |
            std::views::transform(&world::RosterEntry::wage)
        };
        return std::reduce(view.begin(), view.end(), 0.0);
    }

  private:
    world::CompanyBoard&                     companyBoard_;
    std::vector<SafePtr<world::RosterEntry>> emptyRosterPool_;
};

class [[nodiscard]] LaborDemander {
  public:
    LaborDemander(pcg32& masterRng, world::CompanyBoard& companyBoard);
    void post(
        const int                                    id,
        const int                                    desiredEmploy,
        tbb::concurrent_vector<world::LaborRequest>& requestBox
    ) PRE(id > 0) PRE(desiredEmploy > 0) {
        recruiter_.post(id, desiredEmploy, requestBox);
    }
    void offer() { recruiter_.offer(); }
    void layOffs(const int layOffsCnt) PRE(layOffsCnt > 0) { hrManager_.layOffs(layOffsCnt); }
    void registerMember(world::Workspace& workspace);
    void acceptResignation() { hrManager_.acceptResignation(); }
    auto employeeCnt() const -> int POST(cnt : cnt >= 0) { return hrManager_.employeeCnt(); }
    auto sumWage() const -> double POST(wage : wage >= 0.0) { return hrManager_.sumWage(); }
    void endStep(world::CensusDropBox& dropBox) { recruiter_.endStep(dropBox); }

  private:
    Recruiter            recruiter_;
    HumanResourceManager hrManager_;
};
}  // namespace labor_demander