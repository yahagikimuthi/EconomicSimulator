#pragma once

#include "core/values/goods.hpp"
#include "world/message.hpp"

namespace abm::base_goods::supplier {
struct TradePlan {
    Price         price{0.0};
    GoodsQuantity supply{0.0};
};

struct TradeResult {
    GoodsQuantity supply;
    GoodsQuantity soldAmount;
    GoodsQuantity totalDemand;
    Money         sales;
};

class [[nodiscard]] PlanMemory {
  public:
    PlanMemory();
    void memorize(TradePlan& newPlan) { currentPlan_ = newPlan; }
    auto rememberLastPlan() const -> const TradePlan& { return log_; }
    void endStep(CensusDropBox& dropBox) {
        log_ = currentPlan_;
        dropBox.prices.emplace_back(currentPlan_.price.value());
        dropBox.supplies.emplace_back(currentPlan_.supply.value());
        currentPlan_ = {.price = Price{0.0}, .supply = GoodsQuantity{0.0}};
    }

  private:
    TradePlan log_;
    TradePlan currentPlan_;
};

class [[nodiscard]] TradingMemory {
  public:
    TradingMemory();
    void memory(const TradeResult& newResult) { currentResult_ = newResult; }
    auto rememberLastResult() const -> const TradeResult& { return log_; }

  private:
    TradeResult log_;
    TradeResult currentResult_;
};

class [[nodiscard]] Memory {
  public:
    Memory();

  private:
    PlanMemory    planMemory_;
    TradingMemory tradingMemory_;
};
}  // namespace abm::base_goods::supplier