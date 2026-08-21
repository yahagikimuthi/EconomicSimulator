#pragma once

#include <tbb/concurrent_vector.h>
#include <optional>
#include <pcg_random.hpp>
#include <span>
#include <vector>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/ledger.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"
#include "world/goods.hpp"

namespace abm::production_goods::supplier {
class Trader final {
    using Entry        = ProductionGoodsEntry;
    using Request      = ProductionGoodsRequest;
    using TradePlan    = base_goods::supplier::TradePlan;
    using Ledger       = base_goods::supplier::Ledger;
    using Market       = ProductionGoodsMarket;
    using ATradeResult = base_goods::supplier::ATradeResult;
    using TradeResult  = base_goods::supplier::TradeResult;

  public:
    [[nodiscard]] explicit constexpr Trader(RandomGenerator& masterRng) noexcept
        : rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}} {}

    void post(const AgentID id, const TradePlan& plan, Market& market) noexcept {
        ASSERT(plan.supply >= GoodsQuantity{0.0});
        isActive_ = true;
        if (plan.supply == GoodsQuantity{0.0}) return;
        myEntry_ = market.entry(id, plan.price, plan.supply);
        ledger_.makeNewPage(plan.supply);
    }

    void trade() noexcept {
        if (not isPosting()) return;
        auto       requestBox = requestBoxRef();
        const auto demand     = myEntry_->totalDemand();
        if (demand == GoodsQuantity{0.0}) return;
        const auto tradeAmount    = ledger_.canTradeAmount(demand);
        const auto isExcessDemand = ledger_.isExcessDemand(demand);
        isExcessDemand ? performRationedTrade(requestBox) : myEntry_->performFullTrade();
        ledger_.readResult({.price = myEntry_->price, .demand = demand, .salesAmount = tradeAmount}
        );
    }

    [[nodiscard]] auto publishTradeResult() const noexcept -> std::optional<TradeResult> {
        if (not isActive_) return std::nullopt;
        return ledger_.publishResult();
    }

    void reset() noexcept {
        myEntry_.reset();
        ledger_.reset();
        isActive_ = false;
    }

  private:
    [[nodiscard]] auto requestBoxRef() noexcept -> std::span<RefWrap<Request>> {
        ASSERT(isPosting());
        static thread_local auto requestBox = std::vector<RefWrap<Request>>{};
        requestBox.clear();
        myEntry_->packRequest(requestBox);
        return requestBox;
    }

    void performRationedTrade(std::span<RefWrap<Request>> requestBox) noexcept {
        rng_.shuffle(requestBox);
        auto remainAmount = ledger_.inventory();
        for (RefWrap<Request> reqRef : requestBox) {
            auto&      req       = reqRef.get();
            const auto reqAmount = req.amount;
            if (remainAmount <= reqAmount) {
                req.tradeAmount = remainAmount;
                return;
            }
            req.tradeAmount = reqAmount;
            remainAmount -= reqAmount;
        }
    }

    [[nodiscard]] auto      isPosting() const noexcept -> bool { return myEntry_.has_value(); }
    Ledger                  ledger_;
    std::optional<Entry&>   myEntry_{std::nullopt};
    mutable RandomGenerator rng_;
    bool                    isActive_{false};
};
}  // namespace abm::production_goods::supplier