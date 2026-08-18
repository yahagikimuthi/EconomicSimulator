#pragma once

#include <algorithm>
#include <cmath>
#include <limits>
#include <optional>
#include <pcg_random.hpp>

#include "components/base_goods_supplier/common.hpp"
#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"

namespace abm::base_goods::supplier {
class MarkupPlannerMemory {
    template <typename T>
    class Memory {
      public:
        [[nodiscard]] explicit Memory(T l) noexcept : last_{l} {}
        void commit() noexcept {
            if (current_) last_ = current_;
        }
        void clearLog() { last_.reset(); }

        std::optional<T> last_;
        std::optional<T> current_{std::nullopt};
    };

  public:
    [[nodiscard]] explicit MarkupPlannerMemory(RandomGenerator& masterRng) noexcept;
    [[nodiscard]] auto rememberLastSupply() const noexcept -> std::optional<GoodsQuantity> {
        return supply_.last_;
    }
    [[nodiscard]] auto rememberLastSalesAmount() const noexcept -> std::optional<GoodsQuantity> {
        return salesAmount_.last_;
    }

    void listenTradeResult(const TradeResult& result) noexcept;
    void listenTradePlan(const TradePlan& plan) noexcept;
    void clearLog() noexcept { supply_.clearLog(), salesAmount_.clearLog(); }
    void commit() noexcept { supply_.commit(), salesAmount_.commit(); }

  private:
    Memory<GoodsQuantity> supply_;
    Memory<GoodsQuantity> salesAmount_;
};

class MarkupPlanner final {
  public:
    [[nodiscard]] explicit MarkupPlanner(RandomGenerator& masterRng);

    [[nodiscard]] auto plan() noexcept -> double {
        const auto next = calcNextMarkup();
        memory_.clearLog();
        if (not next) return log_;
        log_ = *next;
        return *next;
    }

  private:
    // isSold = (前期供給 - 前期売上) / 前回供給 < 定数
    [[nodiscard]] auto calcNextMarkup() const -> std::optional<double> {
        const auto lastSupply      = memory_.rememberLastSupply();
        const auto lastSalesAmount = memory_.rememberLastSalesAmount();
        if (not lastSupply or not lastSalesAmount) return std::nullopt;
        ASSERT(*lastSupply >= GoodsQuantity{0.0});
        const auto inventory = *lastSupply - *lastSalesAmount;
        ASSERT(inventory >= GoodsQuantity{0.0});
        const auto isSupplied = *lastSupply == GoodsQuantity{0.0};
        const auto isSold     = isSupplied ? inventory / *lastSupply < targetInvRatio_ : true;
        return calcNextMarkup(isSold);
    }

    [[nodiscard]] auto calcNextMarkup(const bool isSold) const -> double {
        const auto alpha      = std::abs(rng_.randNormal(0.0, adjustVol_));
        const auto nextMarkup = log_ + (isSold ? alpha : -alpha);
        return markupGuard(nextMarkup);
    }

    [[nodiscard]] static auto markupGuard(const double markup) noexcept -> double {
        return std::max(markup, std::numeric_limits<double>::epsilon());
    }

    MarkupPlannerMemory     memory_;
    mutable RandomGenerator rng_;
    double                  log_;
    std::optional<double>   plan_;
    const double            adjustVol_;
    const double            targetInvRatio_;
};

class PricePlanner final {
  public:
    [[nodiscard]] explicit PricePlanner(RandomGenerator& masterRng) noexcept;

    [[nodiscard]] auto plan(const GoodsQuantity supply, const double markup, const Money totalCost)
        const noexcept -> Price {
        const auto price = calcPrice(supply, markup, totalCost);
        const auto alpha = rng_.randNormal(0.0, adjustVol_, -1.0, 1.0);
        return price * (1.0 + alpha);
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

    mutable RandomGenerator rng_;
    const double            adjustVol_;
};

class TradePlanner final {
  public:
    [[nodiscard]] explicit TradePlanner(RandomGenerator& masterRng) noexcept;

    [[nodiscard]] auto plan(const GoodsQuantity supply, const Money totalCost) noexcept
        -> TradePlan {
        const auto markup = markupPlanner_.plan();
        const auto price  = pricePlanner_.plan(supply, markup, totalCost);
        return {.price = price, .supply = supply};
    }

  private:
    MarkupPlanner markupPlanner_;
    PricePlanner  pricePlanner_;
};

class EmployPlanner final {
  public:
    [[nodiscard]] explicit EmployPlanner(RandomGenerator& masterRng) noexcept;

  private:
    GoodsQuantity demandForecast_;
};

class Planner {
  public:
    [[nodiscard]] explicit Planner(RandomGenerator& masterRng) noexcept;

    [[nodiscard]] auto planTrading(const GoodsQuantity supply, const Money totalCost) noexcept
        -> TradePlan {
        return tradePlanner_.plan(supply, totalCost);
    }

  private:
    TradePlanner  tradePlanner_;
    EmployPlanner employPlanner_;
};
}  // namespace abm::base_goods::supplier