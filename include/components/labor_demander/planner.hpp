#pragma once

#include <cmath>
#include <pcg_random.hpp>

#include "components/labor_demander/util.hpp"
#include "core/values/labor.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace abm::labor::demander {
[[nodiscard]] inline auto wageGuard(const Wage wage) -> Wage {
    return Wage{std::max(wage.value(), config::labor_demander::epsilonWage)};
}

class [[nodiscard]] WagePlanner {
  public:
    WagePlanner(const pcg32 rng, const Wage log, const double adjustVol)
        : rng_{rng}, log_{log}, adjustVol_{adjustVol} {}

    auto judgeWage(const bool shouldRaise) -> Wage {
        const Wage nextWage{calcWage(shouldRaise)};
        plan_ = nextWage;
        return nextWage;
    }

    void endStep(CensusDropBox& dropBox) {
        dropBox.wages.emplace_back(plan_.value());
        log_  = plan_;
        plan_ = Wage{0.0};
    }

  private:
    auto calcWage(const bool shouldRaise) const -> Wage {
        const double alpha{std::abs(helper::randNormal(rng_, 0.0, adjustVol_, -1.0, 1.0))};
        const Wage   nextWage{log_ * (shouldRaise ? 1.0 + alpha : 1.0 - alpha)};
        return wageGuard(nextWage);
    }

    mutable pcg32 rng_;
    Wage          plan_{0.0};
    Wage          log_;
    const double  adjustVol_;
};

class [[nodiscard]] OfferRateManager {
  public:
    OfferRateManager(const pcg32 rng, const double offerRate, const double adjustVol)
        : rng_{rng}, offerRate_{offerRate}, adjustVol_{adjustVol} {}

    auto offerNum(const HeadCount desiredEmploy) const -> HeadCount {
        const HeadCount offer{desiredEmploy * (1.0 + offerRate_)};
        return HeadCount{
            std::min(static_cast<double>(config::agent_count::hhold), std::ceil(offer.value()))
        };
    }

    void update(const HeadCount employPlan, const HeadCount actualEmploy) {
        offerRate_ = updateOfferRate(employPlan, actualEmploy);
    }

  private:
    auto updateOfferRate(const HeadCount employPlan, const HeadCount actualEmploy) const -> double {
        const double alpha{std::abs(helper::randNormal(rng_, 0.0, adjustVol_, -1.0, 1.0))};
        const bool   shouldRaise{actualEmploy < employPlan};
        const double nextRate{offerRate_ * (shouldRaise ? 1.0 + alpha : 1.0 - alpha)};
        return std::max(1.0, nextRate);
    }

    mutable pcg32 rng_;
    double        offerRate_;
    const double  adjustVol_;
};

class [[nodiscard]] OfferPlanner {
  public:
    OfferPlanner(HeadCount log, const OfferRateManager offerRateManager)
        : log_{log}, offerRateManager_{offerRateManager} {}

    auto judgeOffer(const HeadCount desiredEmploy) -> HeadCount {
        return offerRateManager_.offerNum(desiredEmploy);
    }

    auto shouldRaiseWage(const HeadCount applicantNum) const -> bool { return applicantNum < log_; }

    void endStep(const HeadCount employPlan, const HeadCount actualEmploy) {
        log_ = plan_;
        offerRateManager_.update(employPlan, actualEmploy);
        plan_ = HeadCount{0.0};
    }

  private:
    HeadCount        plan_{0.0};
    HeadCount        log_;
    OfferRateManager offerRateManager_;
};

class [[nodiscard]] PostingInfoPlanner {
  public:
    PostingInfoPlanner(const WagePlanner wagePlanner, const OfferPlanner offerPlanner)
        : wagePlanner_{wagePlanner}, offerPlanner_{offerPlanner} {}

    auto judgePlan(const HeadCount desiredEmploy, const HeadCount lastApplicantNum) -> PostingInfo {
        return {
            .wage     = wagePlanner_.judgeWage(offerPlanner_.shouldRaiseWage(lastApplicantNum)),
            .offerNum = offerPlanner_.judgeOffer(desiredEmploy)
        };
    }

    void endStep(CensusDropBox& dropBox, const HeadCount employPlan, const HeadCount actualEmploy) {
        wagePlanner_.endStep(dropBox);
        offerPlanner_.endStep(employPlan, actualEmploy);
    }

  private:
    WagePlanner  wagePlanner_;
    OfferPlanner offerPlanner_;
};

class [[nodiscard]] RequestPlanner {
  public:
    RequestPlanner(const PostingInfoPlanner postingPlanner) : postingPlanner_{postingPlanner} {}

    auto judgePlan(const HeadCount desiredEmploy, const HeadCount lastApplicantNum) -> PostingInfo {
        employPlan_ = desiredEmploy;
        return postingPlanner_.judgePlan(desiredEmploy, lastApplicantNum);
    }

    void endStep(CensusDropBox& dropBox, const HeadCount actualEmploy)
        PRE(actualEmploy >= HeadCount{0.0}) {
        dropBox.postedEmployments.emplace_back(employPlan_.value());
        postingPlanner_.endStep(dropBox, employPlan_, actualEmploy);
    }

  private:
    PostingInfoPlanner postingPlanner_;
    HeadCount          employPlan_{0.0};
};
}  // namespace abm::labor::demander