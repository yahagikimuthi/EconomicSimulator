#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <pcg_random.hpp>

#include "components/base_goods_supplier/common.hpp"
#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"
#include "world/common.hpp"

namespace abm::base_goods::supplier {
class MarkupPlanner final {
  public:
    [[nodiscard]] MarkupPlanner(const RandomGenerator rng, const double log, const double adjustVol)
        : rng_{rng}, log_{log}, adjustVol_{adjustVol} {}

    [[nodiscard]] auto judgeMarkup(const bool isSold) -> double {
        const auto nextMarkup = calcMarkup(isSold);
        plan_                 = nextMarkup;
        return nextMarkup;
    }

    void endStep(CensusDropBox& dropBox) {
        dropBox.markups.emplace_back(plan_);
        log_  = plan_;
        plan_ = 0.0;
    }

  private:
    [[nodiscard]] auto calcMarkup(const bool isSold) const -> double POST(markup : markup > 0.0) {
        const auto alpha      = std::abs(rng_.randNormal(0.0, adjustVol_));
        const auto nextMarkup = log_ + (isSold ? alpha : -alpha);
        return markupGuard(nextMarkup);
    }

    static auto markupGuard(const double markup) noexcept -> double {
        return std::max(markup, std::numeric_limits<double>::epsilon());
    }

    mutable RandomGenerator rng_;
    double                  log_;
    double                  plan_{};
    const double            adjustVol_;
};

class PricePlanner final {
  public:
    [[nodiscard]] PricePlanner() = default;

    [[nodiscard]] auto judgePrice(
        const GoodsQuantity supply, const double markup, const Money totalCost
    ) -> Price {
        const auto nextPrice = calcPrice(supply, markup, totalCost);
        plan_                = nextPrice;
        return nextPrice;
    }

    void endStep(CensusDropBox& dropBox) {
        dropBox.prices.emplace_back(plan_.value());
        plan_ = Price{0.0};
    }

  private:
    [[nodiscard]] static auto calcPrice(
        const GoodsQuantity supply, const double markup, const Money totalCost
    ) -> Price {
        const auto avgCost =
            Money{(supply != GoodsQuantity{0.0}) ? totalCost.value() / supply.value() : 0.0};
        const auto price = Price{avgCost.value() * (1.0 + markup)};
        return priceGuard(price);
    }

    [[nodiscard]] static auto priceGuard(const Price price) -> Price {
        return Price{std::max(price.value(), std::numeric_limits<double>::epsilon())};
    }

    Price plan_{0.0};
};

class PostingInfoPlanner final {
  public:
    [[nodiscard]] explicit PostingInfoPlanner(MarkupPlanner markupPlanner)
        : markupPlanner_{markupPlanner} {}

    [[nodiscard]] auto judgePlan(
        const GoodsQuantity supply, const Money totalCost, const bool isSold
    ) -> TradePlan {
        const auto markup = markupPlanner_.judgeMarkup(isSold);
        const auto price  = pricePlanner_.judgePrice(supply, markup, totalCost);
        return {.price = price, .markup = markup, .supply = supply};
    }

    void endStep(CensusDropBox& dropBox) {
        markupPlanner_.endStep(dropBox);
        pricePlanner_.endStep(dropBox);
    }

  private:
    MarkupPlanner markupPlanner_;
    PricePlanner  pricePlanner_;
};

class DemandForecastManager final {
  public:
    explicit DemandForecastManager(const double adjustVol) : adjustVol_{adjustVol} {}

    void update(const GoodsQuantity totalDemand) {
        const auto next =
            GoodsQuantity{demandForecast_ + (adjustVol_ * (totalDemand - demandForecast_))};
        demandForecast_ = next;
    }

    [[nodiscard]] auto targetSupply(const double targetInvRatio) const -> GoodsQuantity {
        return demandForecast_ / (1.0 - targetInvRatio);
    }

  private:
    GoodsQuantity demandForecast_{0.0};
    const double  adjustVol_;
};

class MarkupPlannerT {
  public:
    [[nodiscard]] MarkupPlannerT();

  private:
};

class PricePlannerT {};

class PlannerT {
  public:
    PlannerT();

  private:
};

class Planner final {
  public:
    [[nodiscard]] Planner(
        const GoodsQuantity         lastSupply,
        const bool                  isSold,
        const double                targetInvRatio,
        const PostingInfoPlanner    postingPlanner,
        const DemandForecastManager demandForecastManager
    )
        : log_{.supply = lastSupply, .isSold = isSold},
          targetInvRatio_{targetInvRatio},
          postingPlanner_{postingPlanner},
          demandForecastManager_{demandForecastManager} {}

    [[nodiscard]] auto judgePlan(const GoodsQuantity supply, const Money totalCost) -> TradePlan
        PRE(supply >= GoodsQuantity{0.0}) PRE(totalCost >= Money{0.0}) {
        supplyPlan_ = supply;
        return postingPlanner_.judgePlan(supply, totalCost, log_.isSold);
    }

    [[nodiscard]] auto lastSupply() const -> GoodsQuantity POST(supply
                                                                : supply >= GoodsQuantity{0.0}) {
        return log_.supply;
    }

    [[nodiscard]] auto targetSupply() const -> GoodsQuantity POST(target
                                                                  : target >= GoodsQuantity{0.0}) {
        return demandForecastManager_.targetSupply(targetInvRatio_);
    }

    void endStep(
        const GoodsQuantity totalDemand, const GoodsQuantity unsoldAmount, CensusDropBox& dropBox
    ) PRE(totalDemand >= GoodsQuantity{0.0}) PRE(unsoldAmount >= GoodsQuantity{0.0}) {
        dropBox.supplies.emplace_back(supplyPlan_.value());
        postingPlanner_.endStep(dropBox);
        demandForecastManager_.update(totalDemand);
        log_        = {.supply = supplyPlan_, .isSold = isSold(unsoldAmount)};
        supplyPlan_ = GoodsQuantity{0.0};
    }  // namespace goods_supplier

  private:
    [[nodiscard]] auto isSold(const GoodsQuantity unsoldAmount
    ) const -> bool PRE(unsoldAmount >= GoodsQuantity{0.0}) {
        return (supplyPlan_ != GoodsQuantity{0.0}) ? unsoldAmount / supplyPlan_ < targetInvRatio_
                                                   : true;
    }

    GoodsQuantity supplyPlan_{0.0};
    struct {
        GoodsQuantity supply;
        bool          isSold;
    } log_;
    const double          targetInvRatio_;
    PostingInfoPlanner    postingPlanner_;
    DemandForecastManager demandForecastManager_;
};
}  // namespace abm::base_goods::supplier