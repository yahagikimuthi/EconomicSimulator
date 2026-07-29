#include "orchestrator/updates_loggings.hpp"

#include <cmath>
#include <pcg_random.hpp>

#include "components/common.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander.hpp"
#include "core/base.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace firm_finance {
void logging(world::CensusDropBox& dropBox, const Component& comp) { comp.endStep(dropBox); }
}  // namespace firm_finance
namespace hhold_finance {
void logging(world::CensusDropBox& dropBox, const Component& comp) { comp.endStep(dropBox); }
}  // namespace hhold_finance

namespace labor_demander {
auto RequestPlanner::updateOfferRate(const int actualEmploy) const -> double {
    const double alpha{std::abs(helper::randNormal(rng_, 0.0, param_.offerAdjustVol, -1.0, 1.0))};
    const bool   shouldRaise{actualEmploy < plan_.employ};
    const double offerRate{param_.offerRate * (shouldRaise ? 1.0 + alpha : 1.0 - alpha)};
    return std::max(0.0, offerRate);
}

void RequestPlanner::endStep(
    world::CensusDropBox& dropBox, const int actualEmploy, const int applicantNum
) {
    dropBox.postedEmployments.emplace_back(plan_.employ);
    log_ = {
        .wage         = plan_.wage,
        .actualEmploy = actualEmploy,
        .offerPlan    = plan_.offer,
        .applicantNum = applicantNum
    };
    param_.offerRate = updateOfferRate(actualEmploy);
    plan_.reset();
}

void Recruiter::endStep(world::CensusDropBox& dropBox) {
    if (not isRecruiting_) return;
    planner_.endStep(dropBox, ledger_.employing, ledger_.applicantNum);
    myRequest_ = nullptr;
    offerApplicants_.clear();
    isPosting_ = false;
    ledger_.reset();
    isRecruiting_ = false;
}
}  // namespace labor_demander

namespace goods_supplier {
auto Planner::updateDemandForecast(const double totalDemand) const -> double {
    return log_.demandForecast +
           (param_.demandForecastAdjustVol * (totalDemand - log_.demandForecast));
}

auto Planner::isSold(const double unsoldAmount) const -> bool {
    return (plan_.supply != 0.0) ? unsoldAmount / plan_.supply < param_.targetInvRatio : true;
}

void Planner::endStep(
    const double totalDemand, const double unsoldAmount, world::CensusDropBox& dropBox
) {
    dropBox.prices.emplace_back(plan_.price);
    dropBox.supplies.emplace_back(plan_.supply);
    dropBox.markups.emplace_back(plan_.markup);
    log_ = {
        .markup         = plan_.markup,
        .supply         = plan_.supply,
        .demandForecast = updateDemandForecast(totalDemand),
        .isSold         = isSold(unsoldAmount)
    };
    plan_.reset();
}  // namespace goods_supplier

void GoodsSupplier::endStep(world::CensusDropBox& dropBox) {
    planner_.endStep(trader_.totalDemand(), trader_.inventory(), dropBox);
    producer_.endStep(trader_.inventory(), dropBox);
    trader_.endStep();
}
}  // namespace goods_supplier