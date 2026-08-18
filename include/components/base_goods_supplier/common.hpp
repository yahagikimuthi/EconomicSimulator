#pragma once

#include "core/values/goods.hpp"
#include "world/common.hpp"

namespace abm::base_goods::supplier {
struct TradePlan {
    Price         price;
    double        markup;
    GoodsQuantity supply;
};

struct TradeResult {
    GoodsQuantity supply;
    GoodsQuantity soldAmount;
    GoodsQuantity totalDemand;
    Money         sales;
};

class PlanMemory {
  public:
    [[nodiscard]] PlanMemory();
    void memorize(const TradePlan& newPlan) { currentPlan_ = newPlan; }
    auto rememberLastPlan() const -> const TradePlan& { return log_; }
    void endStep(CensusDropBox& dropBox) {
        log_ = currentPlan_;
        dropBox.prices.emplace_back(currentPlan_.price.value());
        dropBox.supplies.emplace_back(currentPlan_.supply.value());
        currentPlan_ = {.price = Price{0.0}, .markup = 0.0, .supply = GoodsQuantity{0.0}};
    }

  private:
    TradePlan log_;
    TradePlan currentPlan_;
};

class TradingMemory {
  public:
    [[nodiscard]] TradingMemory();
    void memorize(const TradeResult& newResult) { currentResult_ = newResult; }
    auto rememberLastResult() const -> const TradeResult& { return log_; }

  private:
    TradeResult log_;
    TradeResult currentResult_;
};

class Memory {
  public:
    [[nodiscard]] Memory();

    void memorize(const TradePlan& plan) { planMemory_.memorize(plan); }
    void memorize(const TradeResult& result) { tradingMemory_.memorize(result); }

  private:
    PlanMemory    planMemory_;
    TradingMemory tradingMemory_;
};
}  // namespace abm::base_goods::supplier