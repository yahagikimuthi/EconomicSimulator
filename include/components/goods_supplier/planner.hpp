#pragma once

#include <cmath>
#include <pcg_random.hpp>

#include "core/base.hpp"
#include "core/values/goods.hpp"
#include "helper.hpp"
#include "world/message.hpp"

namespace goods::supplier::internal {
[[nodiscard]] inline auto markupGuard(const double markup) -> double {
    return std::max(markup, config::goods_supplier::epsilonMarkup);
}
[[nodiscard]] inline auto priceGuard(const Price price) -> Price {
    return Price{std::max(price.value(), config::goods_supplier::epsilonPrice)};
}
}  // namespace goods::supplier::internal

namespace goods::supplier {
class [[nodiscard]] Planner {
  public:
    Planner(
        const pcg32         rng,
        const double        lastMarkup,
        const GoodsQuantity lastSupply,
        const GoodsQuantity demandForecast,
        const bool          isSold,
        const double        targetInvRatio,
        const double        markupAdjustVol,
        const double        demandForecastAdjustVol
    )
        : rng_{rng},
          log_{
              .markup         = lastMarkup,
              .supply         = lastSupply,
              .demandForecast = demandForecast,
              .isSold         = isSold
          },
          param_{
              .targetInvRatio          = targetInvRatio,
              .markupAdjustVol         = markupAdjustVol,
              .demandForecastAdjustVol = demandForecastAdjustVol
          } {}

    void judgePlan(const GoodsQuantity supply, const Money totalCost)
        PRE(supply >= GoodsQuantity{0.0}) PRE(totalCost >= Money{0.0}) {
        const double markup{calcMarkup()};
        const Price  price{judgePrice(supply, markup, totalCost)};
        plan_ = {.markup = markup, .price = price, .supply = supply};
    }

    auto pricePlan() const -> Price POST(price : price > Price{0.0}) { return plan_.price; }

    auto lastSupply() const -> GoodsQuantity POST(supply : supply >= GoodsQuantity{0.0}) {
        return log_.supply;
    }

    auto targetSupply() const -> GoodsQuantity POST(target : target >= GoodsQuantity{0.0}) {
        return log_.demandForecast / (1.0 - param_.targetInvRatio);
    }

    void endStep(
        const GoodsQuantity   totalDemand,
        const GoodsQuantity   unsoldAmount,
        world::CensusDropBox& dropBox
    ) PRE(totalDemand >= GoodsQuantity{0.0}) PRE(unsoldAmount >= GoodsQuantity{0.0}) {
        dropBox.prices.emplace_back(plan_.price.value());
        dropBox.supplies.emplace_back(plan_.supply.value());
        dropBox.markups.emplace_back(plan_.markup);
        log_ = {
            .markup         = plan_.markup,
            .supply         = plan_.supply,
            .demandForecast = updateDemandForecast(totalDemand),
            .isSold         = isSold(unsoldAmount)
        };
        plan_.reset();
    }  // namespace goods_supplier

  private:
    auto calcMarkup() const -> double POST(markup : markup > 0.0) {
        const double alpha{std::abs(helper::randNormal(rng_, 0.0, param_.markupAdjustVol))};
        const double nextMarkup{log_.markup + (log_.isSold ? alpha : -alpha)};
        return internal::markupGuard(nextMarkup);
    }

    auto judgePrice(const GoodsQuantity supply, const double markup, const Money totalCost) const
        -> Price PRE(supply >= GoodsQuantity{0.0}) PRE(markup > 0.0) PRE(totalCost >= Money{0.0})
            POST(price
                 : price > Price{0.0}) {
        const Money avgCost{
            (supply != GoodsQuantity{0.0}) ? totalCost.value() / supply.value() : 0.0
        };
        const Price price{avgCost.value() * (1.0 + markup)};
        return internal::priceGuard(price);
    }

    auto updateDemandForecast(const GoodsQuantity totalDemand) const -> GoodsQuantity
        PRE(totalDemand >= GoodsQuantity{0.0}) {
        return log_.demandForecast +
               (param_.demandForecastAdjustVol * (totalDemand - log_.demandForecast));
    }

    auto isSold(const GoodsQuantity unsoldAmount
    ) const -> bool PRE(unsoldAmount >= GoodsQuantity{0.0}) {
        return (plan_.supply != GoodsQuantity{0.0})
                   ? unsoldAmount / plan_.supply < param_.targetInvRatio
                   : true;
    }

    mutable pcg32 rng_;

    struct {
        double        markup{};
        Price         price{0.0};
        GoodsQuantity supply{0.0};
        void          reset() { markup = 0.0, price = Price{0.0}, supply = GoodsQuantity{0.0}; }
    } plan_{};

    struct {
        double        markup;
        GoodsQuantity supply;
        GoodsQuantity demandForecast;
        bool          isSold;
    } log_;

    struct {
        const double targetInvRatio;
        const double markupAdjustVol;
        const double demandForecastAdjustVol;
    } param_;
};
}  // namespace goods::supplier