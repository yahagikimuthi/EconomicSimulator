#pragma once

#include <concepts>

#include "core/values/common.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

namespace abm::labor::demander {
template <typename F>
concept AddRosterFn = requires(F f, AgentID id, Wage wage) {
    { f(id, wage) } -> std::same_as<RosterEntry&>;
};

struct RecruitPlan {
    Wage      wage;
    HeadCount employ;
    HeadCount offer;
};

struct RecruitResult {
    HeadCount applicants;
    HeadCount employ;
};

class [[nodiscard]] RecruitPlanMemory {
  public:
    RecruitPlanMemory(
        const Wage lastWage, const HeadCount lastEmployPlan, const HeadCount lastOfferPlan
    )
        : lastPlan_{.wage = lastWage, .employ = lastEmployPlan, .offer = lastOfferPlan} {}
    void memorize(const RecruitPlan& newPlan) {
        currentPlan_  = newPlan;
        wasMemorized_ = true;
    }
    auto rememberLog() const -> const RecruitPlan& { return lastPlan_; }
    void endStep(CensusDropBox& dropBox) {
        if (not wasMemorized_) return;
        dropBox.postedEmployments.emplace_back(currentPlan_.employ.value());
        lastPlan_     = currentPlan_;
        currentPlan_  = {.wage = Wage{0.0}, .employ = HeadCount{0.0}, .offer = HeadCount{0.0}};
        wasMemorized_ = false;
    }

  private:
    RecruitPlan lastPlan_;
    RecruitPlan currentPlan_{.wage = Wage{0.0}, .employ = HeadCount{0.0}, .offer = HeadCount{0.0}};
    bool        wasMemorized_{false};
};

class [[nodiscard]] RecruitResultMemory {
  public:
    RecruitResultMemory(const HeadCount lastApplicants, const HeadCount lastEmploy)
        : lastResult_{.applicants = lastApplicants, .employ = lastEmploy} {}
    void memorize(const RecruitResult& newResult) {
        currentResult_ = newResult;
        wasMemorized_  = true;
    }
    auto rememberLog() const -> const RecruitResult& { return lastResult_; }
    void endStep() {
        if (not wasMemorized_) return;
        lastResult_    = currentResult_;
        currentResult_ = {.applicants = HeadCount{0.0}, .employ = HeadCount{0.0}};
        wasMemorized_  = false;
    }

  private:
    RecruitResult lastResult_;
    RecruitResult currentResult_{.applicants = HeadCount{0.0}, .employ = HeadCount{0.0}};
    bool          wasMemorized_{false};
};

class [[nodiscard]] Memory {
  public:
    Memory(const RecruitPlanMemory& planMemory, const RecruitResultMemory& resultMemory)
        : planMemory_{planMemory}, resultMemory_{resultMemory} {}

    void memorize(const RecruitPlan& newPlan) { planMemory_.memorize(newPlan); }
    void memorize(const RecruitResult& newResult) { resultMemory_.memorize(newResult); }
    auto rememberLastRecruitPlan() const -> const RecruitPlan& { return planMemory_.rememberLog(); }
    auto rememberLastRecruitResult() const -> const RecruitResult& {
        return resultMemory_.rememberLog();
    }
    void endStep(CensusDropBox& dropBox) {
        planMemory_.endStep(dropBox);
        resultMemory_.endStep();
    }

  private:
    RecruitPlanMemory   planMemory_;
    RecruitResultMemory resultMemory_;
};
}  // namespace abm::labor::demander