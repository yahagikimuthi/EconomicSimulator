#include "strategies/goods_demander.hpp"
#include "strategies/goods_supplier.hpp"
#include "strategies/labor_demander.hpp"
#include "strategies/labor_supplier.hpp"

// TODO 各エージェントについて資産を追加する処理を追加。
namespace labor_demander {
void reset(Component& comp) {
    comp.log_ = {
        .wage_         = comp.plan_.wage_,
        .targetEmploy_ = comp.plan_.employ_,
        .actualEmploy_ = comp.employmentLedger.employing
    };
    comp.plan_               = {.wage_ = 0.0, .employ_ = 0};
    comp.humanResources      = {.sumWage_ = 0.0, .employeeCnt = 0};
    comp.employmentLedger    = {.employing = 0};
    comp.posting_.myRequest_ = nullptr;
    comp.posting_.isPosting_ = false;
    comp.posting_.offerApplicants_.clear();
}
}  // namespace labor_demander

namespace labor_supplier {
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
void reset(Component& comp) {
    comp.log_ = {
        .markup_ = comp.plan_.markup_,
        .price_  = comp.plan_.price_,
        .supply_ = comp.plan_.supply_,
        .sales_  = comp.salesLedger.currentSales,
        .isSold_ = (comp.salesLedger.currentSales != 0.0)
                       ? (comp.plan_.supply_ * comp.plan_.price_ / comp.salesLedger.currentSales <
                          comp.parameter_.targetInventoryRatio_)
                       : false
    };
    comp.plan_                  = {.markup_ = 0.0, .price_ = 0.0, .supply_ = 0.0};
    comp.production_.inventory_ = comp.salesLedger.inventory_;
    comp.posting_               = {.myEntry_ = nullptr, .isPosting_ = false};
    comp.salesLedger            = {.inventory_ = 0.0, .currentSales = 0.0};
}
}  // namespace goods_supplier