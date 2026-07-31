#pragma once

#include <tbb/concurrent_vector.h>
#include <pcg_random.hpp>

#include "core/base.hpp"
#include "core/forward.hpp"
#include "world/message.hpp"

namespace goods_supplier {
class [[nodiscard]] Planner {
  public:
    Planner(pcg32& masterRng);
    void judgePlan(const double supply, const double totalCost) PRE(supply >= 0.0)
        PRE(totalCost >= 0.0);
    auto pricePlan() const -> double POST(price : price > 0.0) { return plan_.price; }
    auto lastSupply() const -> double POST(supply : supply >= 0.0) { return log_.supply; }
    auto targetSupply() const -> double POST(target : target >= 0.0) {
        return log_.demandForecast / (1.0 - param_.targetInvRatio);
    }
    void endStep(const double totalDemand, const double unsoldAmount, world::CensusDropBox& dropBox)
        PRE(totalDemand >= 0.0) PRE(unsoldAmount >= 0.0);

  private:
    auto calcMarkup() const -> double POST(markup : markup > 0.0);
    auto judgePrice(const double supply, const double markup, const double totalCost) const
        -> double PRE(supply >= 0.0) PRE(markup > 0.0) PRE(totalCost >= 0.0) POST(price
                                                                                  : price > 0.0);
    auto updateDemandForecast(const double totalDemand) const -> double PRE(totalDemand >= 0.0);
    auto isSold(const double unsoldAmount) const -> bool PRE(unsoldAmount >= 0.0);

    mutable pcg32 rng_;
    struct {
        double markup;
        double price;
        double supply;
        void   reset() { markup = 0.0, price = 0.0, supply = 0.0; }
    } plan_{};
    struct {
        double markup;
        double supply;
        double demandForecast;
        bool   isSold;
    } log_;
    struct {
        const double targetInvRatio;
        const double markupAdjustVol;
        const double demandForecastAdjustVol;
    } param_;
};

class Trader {
  public:
    Trader(pcg32& masterRng);
    void post(
        const double                               supply,
        const double                               pricePlan,
        tbb::concurrent_vector<world::GoodsEntry>& entryBox
    );
    void trade();
    auto inventory() const -> double POST(inv : inv >= 0.0) { return ledger_.inventory; }
    auto sales() const -> double POST(sales : sales >= 0.0) { return ledger_.currentSales; }
    auto totalDemand() const -> double POST(demand : demand >= 0.0) { return ledger_.totalDemand; }
    void endStep() { myEntry_ = nullptr, isPosting = false, ledger_.reset(); }

  private:
    pcg32                      rng_;
    SafePtr<world::GoodsEntry> myEntry_{nullptr};
    bool                       isPosting{false};

    struct {
        double inventory;
        double currentSales;
        double totalDemand;
        void   reset() { inventory = 0.0, currentSales = 0.0, totalDemand = 0.0; }
    } ledger_{};
};

class [[nodiscard]] Producer {
  public:
    Producer(pcg32& masterRng, world::Workspace& workspace);
    auto product() const -> double;
    auto calcDesiredEmploy(
        const double targetSupply, const double lastSupply, const int employeeCnt
    ) const -> int PRE(targetSupply >= 0.0) PRE(lastSupply >= 0.0) PRE(employeeCnt >= 0);
    void endStep(const double unsoldAmount, world::CensusDropBox& dropBox)
        PRE(unsoldAmount >= 0.0) {
        inventory_ = unsoldAmount;
        dropBox.inventories.emplace_back(inventory_);
    }
    auto workspace() -> world::Workspace& { return workspace_; }

  private:
    world::Workspace& workspace_;
    const double      firmProductPower_;
    double            inventory_;
};

class [[nodiscard]] GoodsSupplier {
  public:
    GoodsSupplier(pcg32& masterRng, world::Workspace& workspace);
    void post(const double totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox);
    void trade() { trader_.trade(); }
    auto calcDesiredEmploy(const int employeeCnt) const -> int PRE(employeeCnt >= 0)
                                                            POST(employ
                                                                 : employ >= 0);
    void endStep(world::CensusDropBox& dropBox);
    auto sales() const -> double { return trader_.sales(); }
    auto workspace() -> world::Workspace& { return producer_.workspace(); }

  private:
    Planner  planner_;
    Trader   trader_;
    Producer producer_;
};
}  // namespace goods_supplier