#pragma once

#include <tbb/concurrent_vector.h>
#include <concepts>
#include <cstddef>
#include <pcg_random.hpp>
#include <ranges>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace labor::demander {
class [[nodiscard]] RequestPlanner {
  public:
    RequestPlanner(
        const pcg32     rng,
        const Wage      lastWage,
        const HeadCount lastEmploy,
        const HeadCount lastOfferPlan,
        const HeadCount lastApplicantNum,
        const double    offerRate,
        const double    wageAdjustVol,
        const double    offerAdjustVol
    );

    void judgePlan(const HeadCount desiredEmploy);
    auto wagePlan() const -> Wage POST(wage : wage > Wage{0.0}) { return plan_.wage; }
    auto offerPlan() const -> HeadCount POST(employ : employ >= HeadCount{0.0}) {
        return plan_.offer;
    }
    void endStep(
        world::CensusDropBox& dropBox, const HeadCount actualEmploy, const HeadCount applicantNum
    ) PRE(actualEmploy >= HeadCount{0.0}) PRE(applicantNum >= HeadCount{0.0});

  private:
    auto calcNextWage() const -> Wage POST(wage : wage > Wage{0.0});
    auto calcNextOffer(const HeadCount desiredEmploy) const -> HeadCount
        POST(offer
             : offer >= HeadCount{0.0});
    auto updateOfferRate(const HeadCount actualEmploy) const -> double POST(rate : rate > 0.0);

    mutable pcg32 rng_;
    struct {
        Wage      wage{0.0};
        HeadCount employ{0.0};
        HeadCount offer{0.0};
        void      reset() { wage = Wage{0.0}, employ = HeadCount{0.0}, offer = HeadCount{0.0}; }
    } plan_{};

    struct {
        Wage      wage;
        HeadCount actualEmploy;
        HeadCount offerPlan;
        HeadCount applicantNum;
    } log_;

    struct {
        double       offerRate;
        const double wageAdjustVol;
        const double offerAdjustVol;
    } param_;
};

class [[nodiscard]] Recruiter {
  public:
    Recruiter(const RequestPlanner&& planner) : planner_{planner} {}

    void post(
        const AgentID                                id,
        const HeadCount                              desiredEmploy,
        tbb::concurrent_vector<world::LaborRequest>& requestBox
    ) PRE(desiredEmploy >= HeadCount{0.0});
    void offer();
    template <typename F>
        requires requires(F addRoster, AgentID id, Wage wage) {
            { addRoster(id, wage) } -> std::same_as<SafePtr<world::RosterEntry>>;
        }
    void registerMember(F addRoster) {
        using Entry = world::LaborEntry;
        if (not isPosting_) return;

        HeadCount              employCnt{0.0};
        std::ranges::view auto acceptApplicants{
            offerApplicants_ |
            std::views::transform([](SafePtr<Entry> entry) -> Entry& { return *entry; }) |
            std::views::filter(&Entry::isAccept)
        };
        for (auto&& acceptApplicant : acceptApplicants) {
            acceptApplicant.rosterEntry = addRoster(acceptApplicant.hholdID, myRequest_->wage);
            ++employCnt;
        }
        ledger_.employing += employCnt;
    }
    void endStep(world::CensusDropBox& dropBox);

  private:
    RequestPlanner planner_;

    SafePtr<world::LaborRequest>            myRequest_{nullptr};
    std::vector<SafePtr<world::LaborEntry>> offerApplicants_;
    bool                                    isPosting_{false};

    struct {
        HeadCount remainOfferNum{0.0};
        HeadCount applicantNum{0.0};
        HeadCount employing{0.0};
        void      reset() {
            remainOfferNum = HeadCount{0.0}, applicantNum = HeadCount{0.0},
            employing = HeadCount{0.0};
        }
    } ledger_{};

    bool isRecruiting_{false};
};

class [[nodiscard]] HumanResourceManager {
  public:
    HumanResourceManager(world::CompanyBoard& companyBoard) : companyBoard_{companyBoard} {}
    auto addRoster(const AgentID id, const Wage wage, world::Workspace& workspace)
        -> SafePtr<world::RosterEntry> PRE(id >= AgentID{0}) PRE(wage > Wage{0.0});
    void acceptResignation();
    void layOffs(const HeadCount layOffsCnt) PRE(layOffsCnt > HeadCount{0.0});
    auto employeeCnt() const -> HeadCount POST(cnt : cnt >= HeadCount{0.0}) {
        ASSERT(companyBoard_.roster.size() >= emptyRosterPool_.size());
        const std::size_t rosterSize{companyBoard_.roster.size() - emptyRosterPool_.size()};
        return HeadCount{static_cast<double>(rosterSize)};
    }
    auto sumWage() const -> Wage POST(wage : wage >= Wage{0.0}) {
        using Entry        = world::RosterEntry;
        const auto& roster = companyBoard_.roster;
        return Wage{std::ranges::fold_left(
            roster | std::views::filter([](const Entry& entry) -> bool {
                return entry.isOccupied;
            }) | std::views::transform([](const Entry& entry) -> double {
                return entry.wage.value();
            }),
            0.0,
            std::plus{}
        )};
    };
    void endStep();

  private:
    world::CompanyBoard&                     companyBoard_;
    std::vector<SafePtr<world::RosterEntry>> emptyRosterPool_;
};

class [[nodiscard]] LaborDemander {
  public:
    LaborDemander(const Recruiter&& recruiter, const HumanResourceManager&& hrManager)
        : recruiter_{recruiter}, hrManager_{hrManager} {}
    void post(
        const AgentID                                id,
        const HeadCount                              desiredEmploy,
        tbb::concurrent_vector<world::LaborRequest>& requestBox
    ) PRE(desiredEmploy > HeadCount{0.0}) {
        recruiter_.post(id, desiredEmploy, requestBox);
    }
    void offer() { recruiter_.offer(); }
    void layOffs(const HeadCount layOffsCnt) PRE(layOffsCnt > HeadCount{0.0}) {
        hrManager_.layOffs(layOffsCnt);
    }
    void registerMember(world::Workspace& workspace) {
        recruiter_.registerMember(
            [&](const AgentID id, const Wage wage) -> SafePtr<world::RosterEntry> {
                return hrManager_.addRoster(id, wage, workspace);
            }
        );
    };
    void acceptResignation() { hrManager_.acceptResignation(); }
    auto employeeCnt() const -> HeadCount POST(cnt : cnt >= HeadCount{0.0}) {
        return hrManager_.employeeCnt();
    }
    auto sumWage() const -> Money POST(wage : wage >= Money{0.0}) {
        return static_cast<Money>(hrManager_.sumWage());
    }
    void endStep(world::CensusDropBox& dropBox) {
        recruiter_.endStep(dropBox);
        hrManager_.endStep();
    }

  private:
    Recruiter            recruiter_;
    HumanResourceManager hrManager_;
};
}  // namespace labor::demander