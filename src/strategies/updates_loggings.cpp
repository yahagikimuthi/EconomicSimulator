#include "strategies/updates_loggings.hpp"

#include <cmath>
#include <pcg_random.hpp>

#include "components/common.hpp"
#include "components/goods_demander.hpp"
#include "components/goods_supplier.hpp"
#include "components/labor_demander.hpp"
#include "components/labor_supplier.hpp"
#include "core/base.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace firm_finance {
void logging(world::CensusDropBox& dropBox, const Component& comp) {
    dropBox.firmAssets.emplace_back(comp.asset_);
}
}  // namespace firm_finance
namespace hhold_finance {
void logging(world::CensusDropBox& dropBox, const Component& comp) {
    dropBox.hholdAssets.emplace_back(comp.asset_);
}
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

namespace labor_supplier {
void logging(world::CensusDropBox& dropBox, const Component& comp) {
    if (not comp.rosterEntry_) return;
    dropBox.wages.emplace_back(comp.rosterEntry_->wage);
}
void reset(Component& comp) {
    comp.posting_.myEntries_.clear();
    comp.posting_.acceptEntry_ = nullptr;
    comp.posting_.isPosting_   = false;
}
}  // namespace labor_supplier

namespace goods_supplier {
void logging(world::CensusDropBox& dropBox, const Component& comp) {
    dropBox.prices.emplace_back(comp.plan_.price_);
    dropBox.supplies.emplace_back(comp.plan_.supply_);
    dropBox.markups.emplace_back(comp.plan_.markup_);
    dropBox.inventories.emplace_back(comp.production_.inventory_);
}
void reset(Component& comp) {
    comp.log_ = {
        .markup_         = comp.plan_.markup_,
        .supply_         = comp.plan_.supply_,
        .demandForecast_ = comp.log_.demandForecast_ +
                           (comp.parameter_.demandForecastAdjustmentParam_ *
                            (comp.salesLedger.totalDemand_ - comp.log_.demandForecast_)),
        .isSold_ = (comp.plan_.supply_ != 0.0) ? comp.salesLedger.inventory_ / comp.plan_.supply_ <
                                                     comp.parameter_.targetInventoryRatio_
                                               : true
    };
    comp.plan_                                  = {.markup_ = 0.0, .price_ = 0.0, .supply_ = 0.0};
    comp.production_.workspace_.totalLaborInput = 0.0;
    comp.production_.inventory_                 = comp.salesLedger.inventory_;
    comp.posting_                               = {.myEntry_ = nullptr, .isPosting_ = false};
    comp.salesLedger = {.inventory_ = 0.0, .currentSales_ = 0.0, .totalDemand_ = 0.0};
}
}  // namespace goods_supplier