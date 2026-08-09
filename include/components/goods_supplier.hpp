#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>
#include <pcg_random.hpp>

#include "core/base.hpp"
#include "core/forward.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "core/values/labor.hpp"
#include "world/message.hpp"

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
    );
    void judgePlan(const GoodsQuantity supply, const Money totalCost)
        PRE(supply >= GoodsQuantity{0.0}) PRE(totalCost >= Money{0.0});
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
    ) PRE(totalDemand >= GoodsQuantity{0.0}) PRE(unsoldAmount >= GoodsQuantity{0.0});

  private:
    auto calcMarkup() const -> double POST(markup : markup > 0.0);
    auto judgePrice(const GoodsQuantity supply, const double markup, const Money totalCost) const
        -> Price PRE(supply >= GoodsQuantity{0.0}) PRE(markup > 0.0) PRE(totalCost >= Money{0.0})
            POST(price
                 : price > Price{0.0});
    auto updateDemandForecast(const GoodsQuantity totalDemand) const -> GoodsQuantity
        PRE(totalDemand >= GoodsQuantity{0.0});
    auto isSold(const GoodsQuantity unsoldAmount
    ) const -> bool PRE(unsoldAmount >= GoodsQuantity{0.0});

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

class Trader {
  public:
    Trader(const pcg32 rng) : rng_{rng} {}
    void post(
        const GoodsQuantity                        supply,
        const Price                                pricePlan,
        tbb::concurrent_vector<world::GoodsEntry>& entryBox
    );
    void trade();
    auto inventory() const -> GoodsQuantity POST(inv : inv >= GoodsQuantity{0.0}) {
        return ledger_.inventory;
    }
    auto sales() const -> Money POST(sales : sales >= Money{0.0}) { return ledger_.currentSales; }
    auto totalDemand() const -> GoodsQuantity POST(demand : demand >= GoodsQuantity{0.0}) {
        return ledger_.totalDemand;
    }
    void endStep() { myEntry_.reset(), isPosting_ = false, ledger_.reset(); }

  private:
    pcg32                             rng_;
    std::optional<world::GoodsEntry&> myEntry_{std::nullopt};
    bool                              isPosting_{false};

    struct {
        GoodsQuantity inventory{0.0};
        Money         currentSales{0.0};
        GoodsQuantity totalDemand{0.0};

        void reset() {
            inventory = GoodsQuantity{0.0}, currentSales = Money{0.0},
            totalDemand = GoodsQuantity{0.0};
        }
    } ledger_{};
};

class [[nodiscard]] Producer {
  public:
    Producer(
        world::Workspace& workspace, const double firmProductPower, const GoodsQuantity inventory
    )
        : workspace_{workspace}, firmProductPower_{firmProductPower}, inventory_{inventory} {}
    auto product() const -> GoodsQuantity;
    auto calcDesiredEmploy(
        const GoodsQuantity targetSupply,
        const GoodsQuantity lastSupply,
        const HeadCount     employeeCnt
    ) const -> HeadCount PRE(targetSupply >= GoodsQuantity{0.0})
                PRE(lastSupply >= GoodsQuantity{0.0}) PRE(employeeCnt >= HeadCount{0.0});
    void endStep(const GoodsQuantity unsoldAmount, world::CensusDropBox& dropBox)
        PRE(unsoldAmount >= GoodsQuantity{0.0}) {
        inventory_ = unsoldAmount;
        dropBox.inventories.emplace_back(inventory_.value());
        workspace_.resetInput();
    }
    auto workspace() -> world::Workspace& { return workspace_; }

  private:
    world::Workspace& workspace_;
    const double      firmProductPower_;
    GoodsQuantity     inventory_;
};

class [[nodiscard]] GoodsSupplier {
  public:
    GoodsSupplier(const Planner&& planner, const Trader&& trader, const Producer&& producer)
        : planner_{planner}, trader_{trader}, producer_{producer} {}
    void post(const Money totalCost, tbb::concurrent_vector<world::GoodsEntry>& entryBox);
    void trade() { trader_.trade(); }
    auto calcDesiredEmploy(const HeadCount employeeCnt) const -> HeadCount
        PRE(employeeCnt >= HeadCount{0.0}) POST(employ
                                                : employ >= HeadCount{0.0});
    void endStep(world::CensusDropBox& dropBox);
    auto sales() const -> Money { return trader_.sales(); }
    auto workspace() -> world::Workspace& { return producer_.workspace(); }

  private:
    Planner  planner_;
    Trader   trader_;
    Producer producer_;
};
}  // namespace goods::supplier