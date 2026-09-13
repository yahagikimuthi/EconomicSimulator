#pragma once

#include <algorithm>
#include <cassert>
#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

#include "components/base_goods_supplier/common.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "world/base_goods.hpp"

namespace abm::base_goods::supplier {

class Trader final {
  public:
    explicit Trader(RandomGenerator& masterRng) noexcept
        : rng_{{masterRng.makeUint64(), masterRng.makeUint64()}} {}

    void post(const AgentID id, const TradePlan& plan, Market& market) noexcept {
        assert(plan.supply.isZeroOrMore());
        if (plan.supply.isZero()) return;
        myEntry_ = market.entry(id, plan.price, plan.supply);
    }

    [[nodiscard]] auto trade() noexcept -> TradeResult {
        if (not isPosting()) return makeDefaultResult();
        const auto demand = calcTotalDemand();
        if (demand.isZero()) return makeDefaultResult(myEntry_->supply);

        const auto isExcessDemand = demand > myEntry_->supply;
        const auto result =
            isExcessDemand ? performRationedTrade(demand) : performFullTrade(demand);

        myEntry_.reset();
        return result;
    }

  private:
    [[nodiscard]] static auto makeDefaultResult(
        const GoodsQuantity supply = GoodsQuantity{0.0}
    ) noexcept -> TradeResult {
        return {
            .soldAmount   = GoodsQuantity{0.0},
            .unsoldAmount = supply,
            .totalDemand  = GoodsQuantity{0.0},
            .sales        = Money{0.0}
        };
    }

    [[nodiscard]] auto calcTotalDemand() const noexcept -> GoodsQuantity {
        const auto requests = myEntry_->requests();
        return std::ranges::fold_left(
            requests | std::views::transform([&](const Request& req) noexcept -> GoodsQuantity {
                return req.payment / myEntry_->price;
            }),
            GoodsQuantity{0.0},
            std::plus{}
        );
    }

    [[nodiscard]] auto performRationedTrade(const GoodsQuantity demand) noexcept -> TradeResult {
        auto requests = packRequest();
        rng_.shuffle(requests);

        auto remainAmount = myEntry_->supply;
        auto totalSales   = Money{0.0};
        for (auto reqRef : requests) {
            auto&      req       = reqRef.get();
            const auto reqAmount = req.payment / myEntry_->price;
            if (remainAmount <= reqAmount) {
                totalSales += req.trade(remainAmount);
                return {
                    .soldAmount   = myEntry_->supply,
                    .unsoldAmount = GoodsQuantity{0.0},
                    .totalDemand  = demand,
                    .sales        = totalSales
                };
            }
            totalSales += req.trade(reqAmount);
            remainAmount -= reqAmount;
        }
        assert(false);
        std::unreachable();
    }

    [[nodiscard]] auto isPosting() const noexcept -> bool { return myEntry_.has_value(); }

    [[nodiscard]] auto packRequest() noexcept -> std::span<Ref<Request>> {
        static thread_local auto refs = std::vector<Ref<Request>>{};
        refs.clear();
        auto requests = myEntry_->requests();
        refs.reserve(requests.size());
        for (auto& req : requests) refs.emplace_back(std::ref(req));
        return refs;
    }

    [[nodiscard]] auto performFullTrade(const GoodsQuantity demand) noexcept -> TradeResult {
        auto sumSales = Money{0.0};
        for (auto& request : myEntry_->requests()) {
            const auto tradeAmount = request.payment / myEntry_->price;
            sumSales += request.trade(tradeAmount);
        }
        return {
            .soldAmount   = demand,
            .unsoldAmount = myEntry_->supply - demand,
            .totalDemand  = demand,
            .sales        = sumSales
        };
    }

    std::optional<Entry&>   myEntry_{std::nullopt};
    mutable RandomGenerator rng_;
};
}  // namespace abm::base_goods::supplier