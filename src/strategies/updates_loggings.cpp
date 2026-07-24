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
    dropBox.firmAssets_.emplace_back(comp.asset_);
}
}  // namespace firm_finance
namespace hhold_finance {
void logging(world::CensusDropBox& dropBox, const Component& comp) {
    dropBox.hholdAssets_.emplace_back(comp.asset_);
}
}  // namespace hhold_finance

namespace labor_demander {
namespace {
struct [[nodiscard]] UpdateAcceptanceRateView final : BaseView<Component> {
    using BaseView<Component>::BaseView;
    auto offerRate() const -> double { return comp_.parameter_.offerRate_; }
    auto offerAdjustmentVolatility() const -> double {
        return comp_.parameter_.offerAdjustmentVolatility_;
    }
    auto targetEmploy() const -> double { return comp_.plan_.employ_; }
    auto actualEmploy() const -> double { return comp_.employmentLedger.employing_; }
    auto rng() -> pcg32& { return comp_.rng_; }
};

[[nodiscard]] auto updateAcceptanceRate(UpdateAcceptanceRateView view) -> double {
    const double alpha{
        std::abs(helper::randNormal(view.rng(), 0.0, view.offerAdjustmentVolatility()))
    };
    const bool   shouldRaise{view.actualEmploy() < view.targetEmploy()};
    const double offerRate{view.offerRate() * (shouldRaise ? 1.0 + alpha : 1.0 - alpha)};
    return std::max(0.0, offerRate);
}
}  // namespace

void logging(world::CensusDropBox& dropBox, const Component& comp) {
    dropBox.postedEmployments_.emplace_back(comp.plan_.employ_);
    dropBox.employments_.emplace_back(comp.employmentLedger.employing_);
}
void reset(Component& comp) {
    if (comp.plan_.isRecruiting) {
        comp.log_ = {
            .wage_         = comp.plan_.wage_,
            .actualEmploy_ = comp.employmentLedger.employing_,
            .offerPlan_    = comp.plan_.offer_,
            .applicantNum_ = comp.employmentLedger.applicantNum_
        };
        comp.parameter_.offerRate_ = updateAcceptanceRate(UpdateAcceptanceRateView{comp});
    }

    comp.plan_ = {.isRecruiting = false, .wage_ = 0.0, .employ_ = 0, .offer_ = 0};
    comp.humanResources_.sumWage_ = comp.employmentLedger.sumWage_;
    comp.employmentLedger         = {.applicantNum_ = 0, .employing_ = 0, .sumWage_ = 0.0};
    comp.posting_.myRequest_      = nullptr;
    comp.posting_.isPosting_      = false;
    comp.posting_.offerApplicants_.clear();
}
}  // namespace labor_demander

namespace labor_supplier {
void logging(world::CensusDropBox& dropBox, const Component& comp) {
    if (not comp.rosterEntry_) return;
    dropBox.wages_.emplace_back(comp.rosterEntry_->wage_);
}
void reset(Component& comp) {
    comp.posting_.myEntries_.clear();
    comp.posting_.isPosting_ = false;
}
}  // namespace labor_supplier

namespace goods_demander {
void reset(Component& comp) {
    comp.posting_    = {.myRequest_ = nullptr, .isPosting_ = false};
    comp.purchasing_ = {.purchase_ = 0.0};
}
}  // namespace goods_demander

namespace goods_supplier {
void logging(world::CensusDropBox& dropBox, const Component& comp) {
    dropBox.prices_.emplace_back(comp.plan_.price_);
    dropBox.supplies_.emplace_back(comp.plan_.supply_);
    dropBox.markups_.emplace_back(comp.plan_.markup_);
    dropBox.inventories_.emplace_back(comp.production_.inventory_);
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
    comp.plan_                  = {.markup_ = 0.0, .price_ = 0.0, .supply_ = 0.0};
    comp.production_.inventory_ = comp.salesLedger.inventory_;
    comp.posting_               = {.myEntry_ = nullptr, .isPosting_ = false};
    comp.salesLedger            = {.inventory_ = 0.0, .currentSales_ = 0.0, .totalDemand_ = 0.0};
}
}  // namespace goods_supplier