#pragma once

#include <tbb/concurrent_vector.h>

#include "components/goods_supplier.hpp"
#include "config/init_setup.hpp"

namespace goods_supplier {
struct CalcSupplyCtx {
    CalcSupplyCtx(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto firmProductPower() const -> double {
        return comp_.production_.firmProductPower_;
    }
    [[nodiscard]] auto sumEmployeeProductPower() const -> double {
        return comp_.production_.sumEmployeeProductPower_;
    }
    [[nodiscard]] auto inventory() const -> double { return comp_.production_.inventory_; }

  private:
    Component& comp_;
};
[[nodiscard]] auto calcSupply(const CalcSupplyCtx ctx) -> double;

struct CalcMarkupCtx {
    CalcMarkupCtx(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto markupAdjustVol() const -> double {
        return comp_.parameter_.markupAdjustmentVolatility_;
    }
    [[nodiscard]] auto lastMarkup() const -> double { return comp_.log_.markup_; }
    [[nodiscard]] auto isSold() const -> bool { return comp_.log_.isSold_; }

  private:
    Component& comp_;
};
[[nodiscard]] auto calcMarkup(
    const CalcMarkupCtx ctx, const double epsilonMarkup = config::goods_supplier::epsilonMarkup
) -> double;

[[nodiscard]] auto judgePrice(
    const double markup,
    const double totalCost,
    const double epsilonPrice = config::goods_supplier::epsilonPrice
) -> double;

[[nodiscard]] auto calcTotalDemand(const tbb::concurrent_vector<world::GoodsRequest>& requestBox)
    -> double;

[[nodiscard]] auto performRationedTrade(
    const double supply, tbb::concurrent_vector<world::GoodsRequest>& requestBox
) -> double;

[[nodiscard]] auto performFullTrade(tbb::concurrent_vector<world::GoodsRequest>& requestBox)
    -> double;

struct UpdateLogCtx {
    UpdateLogCtx(Component& comp) : comp_{comp} {}

    [[nodiscard]] auto getTargetInvRatio() const -> double {
        return comp_.parameter_.targetInventoryRatio_;
    }

    [[nodiscard]] auto inventory() const -> double { return comp_.production_.inventory_; }
    [[nodiscard]] auto supply() const -> double { return comp_.plan_.supply_; }
    [[nodiscard]] auto price() const -> double { return comp_.plan_.price_; }

    void updateLog(const double price, const double sales, const bool isSold) {
        auto& log  = comp_.log_;
        log.price_ = price, log.sales_ = sales, log.isSold_ = isSold;
        auto& plan  = comp_.plan_;
        log.markup_ = plan.markup_, log.supply_ = plan.supply_;
    }

  private:
    Component& comp_;
};

void updateLog(UpdateLogCtx ctx);
}  // namespace goods_supplier