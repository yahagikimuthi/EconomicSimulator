#pragma once

#include <tbb/concurrent_vector.h>
#include <cstdint>
#include <pcg_random.hpp>

#include "core/base.hpp"
#include "core/forward.hpp"
#include "world/message.hpp"

namespace goods_supplier {
class [[nodiscard]] Planner {
  public:
    Planner(pcg32& masterRng);
    void judgePlan(const double supply, const double totalCost);
    auto pricePlan() const -> double { return plan_.price; }
    auto demandForecast() const -> double { return log_.demandForecast; }
    auto lastSupply() const -> double { return log_.supply; }
    auto targetInvRatio() const -> double { return param_.targetInvRatio; }
    void endStep(
        const double totalDemand, const double unsoldAmount, world::CensusDropBox& dropBox
    );

  private:
    auto calcMarkup() const -> double;
    auto judgePrice(const double supply, const double markup, const double totalCost) const
        -> double;
    auto updateDemandForecast(const double totalDemand) const -> double;
    auto isSold(const double unsoldAmount) const -> bool;

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
    auto inventory() const -> double { return ledger_.inventory; }
    auto sales() const -> double { return ledger_.currentSales; }
    auto totalDemand() const -> double { return ledger_.totalDemand; }
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
        const double demandForecast,
        const double lastSupply,
        const double targetInvRatio,
        const int    employeeCnt
    ) const -> int;
    void endStep(const double unsoldAmount, world::CensusDropBox& dropBox) {
        inventory_ = unsoldAmount;
        dropBox.inventories.emplace_back(inventory_);
    }
    auto workspace() -> world::Workspace& { return workspace_; }

  private:
    auto calcTargetProduction(const double demandForecast, const double targetInvRatio) const
        -> double;

    world::Workspace& workspace_;
    const double      firmProductPower_;
    double            inventory_;
};

class [[nodiscard]] GoodsSupplier {
  public:
    GoodsSupplier(pcg32& masterRng, world::Workspace& workspace);
    void post(const double totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox);
    void trade() { trader_.trade(); }
    auto calcDesiredEmploy(const int employeeCnt) const -> int;
    void endStep(world::CensusDropBox& dropBox);
    auto sales() const -> double { return trader_.sales(); }
    auto workspace() -> world::Workspace& { return producer_.workspace(); }

  private:
    Planner  planner_;
    Trader   trader_;
    Producer producer_;
};
}  // namespace goods_supplier