#include "components/common.hpp"
#include "strategies/common.hpp"
#include "strategies/goods_demander.hpp"
#include "strategies/goods_supplier.hpp"
#include "strategies/labor_demander.hpp"
#include "strategies/labor_supplier.hpp"
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
void logging(world::CensusDropBox& dropBox, const Component& comp) {
    dropBox.postedEmployments_.emplace_back(comp.plan_.employ_);
    dropBox.employments_.emplace_back(comp.employmentLedger.employing_);
}
void reset(Component& comp) {
    comp.log_ = {
        .wage_         = comp.plan_.wage_,
        .targetEmploy_ = comp.plan_.employ_,
        .actualEmploy_ = comp.employmentLedger.employing_
    };
    comp.plan_               = {.wage_ = 0.0, .employ_ = 0};
    comp.humanResources      = {.sumWage_ = 0.0, .employeeCnt = 0};
    comp.employmentLedger    = {.employing_ = 0, .sumWage_ = 0.0};
    comp.posting_.myRequest_ = nullptr;
    comp.posting_.isPosting_ = false;
    comp.posting_.offerApplicants_.clear();
}
}  // namespace labor_demander

namespace labor_supplier {
void logging(world::CensusDropBox& dropBox, const Component& comp) {
    dropBox.wages_.emplace_back(comp.contraction_.wage_);
}
void reset(Component& comp) {
    comp.posting_.myEntries_.clear();
    comp.posting_.isPosting_ = false;
    comp.contraction_        = {.firmID_ = -1, .wage_ = 0.0};
}
}  // namespace labor_supplier

namespace goods_demander {
void reset(Component& comp) {
    comp.posting_    = {.myRequest_ = {nullptr, nullptr}, .isPosting_ = false};
    comp.purchasing_ = {.purchase_ = 0.0};
}
}  // namespace goods_demander

namespace goods_supplier {
void logging(world::CensusDropBox& dropBox, const Component& comp) {
    dropBox.prices_.emplace_back(comp.plan_.price_);
    dropBox.supplies_.emplace_back(comp.plan_.supply_);
    dropBox.markups_.emplace_back(comp.plan_.markup_);
    dropBox.inventory_.emplace_back(comp.production_.inventory_);
}
void reset(Component& comp) {
    comp.log_ = {
        .markup_ = comp.plan_.markup_,
        .price_  = comp.plan_.price_,
        .supply_ = comp.plan_.supply_,
        .sales_  = comp.salesLedger.currentSales,
        .isSold_ = (comp.salesLedger.currentSales != 0.0)
                       ? (comp.salesLedger.inventory_ / comp.plan_.supply_ <
                          comp.parameter_.targetInventoryRatio_)
                       : false
    };
    comp.plan_                  = {.markup_ = 0.0, .price_ = 0.0, .supply_ = 0.0};
    comp.production_.inventory_ = comp.salesLedger.inventory_;
    comp.posting_               = {.myEntry_ = nullptr, .isPosting_ = false};
    comp.salesLedger            = {.inventory_ = 0.0, .currentSales = 0.0};
}
}  // namespace goods_supplier