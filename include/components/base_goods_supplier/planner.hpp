#pragma once

#include <cmath>
#include <pcg_random.hpp>

#include "components/base_goods_supplier/common.hpp"
#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace abm::base_goods::supplier {
[[nodiscard]] inline auto markupGuard(const double markup) -> double {
    return std::max(markup, config::goods_supplier::epsilonMarkup);
}

[[nodiscard]] inline auto priceGuard(const Price price) -> Price {
    return Price{std::max(price.value(), config::goods_supplier::epsilonPrice)};
}

class [[nodiscard]] MarkupPlanner {
  public:
    MarkupPlanner(const pcg32 rng, const double log, const double adjustVol)
        : rng_{rng}, log_{log}, adjustVol_{adjustVol} {}

    auto judgeMarkup(const bool isSold) -> double {
        const double nextMarkup{calcMarkup(isSold)};
        plan_ = nextMarkup;
        return nextMarkup;
    }

    void endStep(CensusDropBox& dropBox) {
        dropBox.markups.emplace_back(plan_);
        log_  = plan_;
        plan_ = 0.0;
    }

  private:
    auto calcMarkup(const bool isSold) const -> double POST(markup : markup > 0.0) {
        const double alpha{std::abs(helper::randNormal(rng_, 0.0, adjustVol_))};
        const double nextMarkup{log_ + (isSold ? alpha : -alpha)};
        return markupGuard(nextMarkup);
    }

    mutable pcg32 rng_;
    double        log_;
    double        plan_{};
    const double  adjustVol_;
};

class [[nodiscard]] PricePlanner {
  public:
    PricePlanner() = default;

    auto judgePrice(const GoodsQuantity supply, const double markup, const Money totalCost)
        -> Price {
        const Price nextPrice{calcPrice(supply, markup, totalCost)};
        plan_ = nextPrice;
        return nextPrice;
    }

    void endStep(CensusDropBox& dropBox) {
        dropBox.prices.emplace_back(plan_.value());
        plan_ = Price{0.0};
    }

  private:
    auto calcPrice(const GoodsQuantity supply, const double markup, const Money totalCost) const
        -> Price {
        const Money avgCost{
            (supply != GoodsQuantity{0.0}) ? totalCost.value() / supply.value() : 0.0
        };
        const Price price{avgCost.value() * (1.0 + markup)};
        return priceGuard(price);
    }

    Price plan_{0.0};
};

class PostingInfoPlanner {
  public:
    PostingInfoPlanner(MarkupPlanner markupPlanner) : markupPlanner_{markupPlanner} {}

    auto judgePlan(const GoodsQuantity supply, const Money totalCost, const bool isSold)
        -> PostingInfo {
        const double markup{markupPlanner_.judgeMarkup(isSold)};
        const Price  price{pricePlanner_.judgePrice(supply, markup, totalCost)};
        return {.price = price, .supply = supply};
    }

    void endStep(CensusDropBox& dropBox) {
        markupPlanner_.endStep(dropBox);
        pricePlanner_.endStep(dropBox);
    }

  private:
    MarkupPlanner markupPlanner_;
    PricePlanner  pricePlanner_;
};

class [[nodiscard]] DemandForecastManager {
  public:
    DemandForecastManager(const double adjustVol) : adjustVol_{adjustVol} {}

    void update(const GoodsQuantity totalDemand) {
        const GoodsQuantity next{demandForecast_ + (adjustVol_ * (totalDemand - demandForecast_))};
        demandForecast_ = next;
    }

    auto targetSupply(const double targetInvRatio) const -> GoodsQuantity {
        return demandForecast_ / (1.0 - targetInvRatio);
    }

  private:
    GoodsQuantity demandForecast_{0.0};
    const double  adjustVol_;
};

class [[nodiscard]] Planner {
  public:
    Planner(
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

    auto judgePlan(const GoodsQuantity supply, const Money totalCost) -> PostingInfo
        PRE(supply >= GoodsQuantity{0.0}) PRE(totalCost >= Money{0.0}) {
        supplyPlan_ = supply;
        return postingPlanner_.judgePlan(supply, totalCost, log_.isSold);
    }

    auto lastSupply() const -> GoodsQuantity POST(supply : supply >= GoodsQuantity{0.0}) {
        return log_.supply;
    }

    auto targetSupply() const -> GoodsQuantity POST(target : target >= GoodsQuantity{0.0}) {
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
    auto isSold(const GoodsQuantity unsoldAmount
    ) const -> bool PRE(unsoldAmount >= GoodsQuantity{0.0}) {
        return (supplyPlan_ != GoodsQuantity{0.0}) ? unsoldAmount / supplyPlan_ < targetInvRatio_
                                                   : true;
    }

    GoodsQuantity supplyPlan_{0.0};

    struct {
        GoodsQuantity supply;
        bool          isSold;
    } log_;

    const double targetInvRatio_;

    PostingInfoPlanner    postingPlanner_;
    DemandForecastManager demandForecastManager_;
};
}  // namespace abm::base_goods::supplier