#pragma once

#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

#include "components/base_goods_supplier/common.hpp"
#include "components/base_goods_supplier/ledger.hpp"
#include "components/common.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "world/base_goods.hpp"
#include "world/common.hpp"

namespace abm::base_goods::supplier {

template <EMarket SupplyGoodsT>
class Trader final {
    using MarketT  = Market<SupplyGoodsT>;
    using RequestT = Request<SupplyGoodsT>;
    using EntryT   = Entry<SupplyGoodsT>;

  public:
    explicit Trader(RandomGenerator& masterRng) noexcept
        : rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}} {}

    void post(const AgentID id, const TradePlan& plan, MarketT& market) noexcept {
        ASSERT(plan.supply.isZeroOrMore());
        if (plan.supply.isZero()) return;
        myEntry_ = market.entry(id, plan.price, plan.supply);
        ledger_.makeNewPage(plan.supply);
    }

    template <DepositFn F>
    void trade(F&& depositFn) noexcept {
        if (not isPosting()) return;
        const auto demand = calcTotalDemand();
        if (demand.isZero()) return;
        const auto tradeAmount    = ledger_.tradableAmount(demand);
        const auto isExcessDemand = ledger_.isExcessDemand(demand);
        isExcessDemand ? performRationedTrade(std::forward<F>(depositFn))
                       : performFullTrade(std::forward<F>(depositFn));
        ledger_.readResult({.price = myEntry_->price, .demand = demand, .salesAmount = tradeAmount}
        );
    }

    [[nodiscard]] auto publishTradeResult() const noexcept -> TradeResult {
        return ledger_.publishResult();
    }

    void reset() noexcept {
        ASSERT(myEntry_);
        myEntry_->disable();
        myEntry_.reset();
        ledger_.reset();
    }

  private:
    [[nodiscard]] auto calcTotalDemand() const noexcept -> GoodsQuantity {
        const auto requests = myEntry_->requests();
        return std::ranges::fold_left(
            requests | std::views::transform([&](const RequestT& req) noexcept -> GoodsQuantity {
                return req.payment() / myEntry_->price;
            }),
            GoodsQuantity{0.0},
            std::plus{}
        );
    }

    void performRationedTrade(DepositFn auto&& depositFn) noexcept {
        auto requests = packRequest();
        rng_.shuffle(requests);

        auto remainAmount = ledger_.inventory();
        for (auto reqRef : requests) {
            auto&      req       = reqRef.get();
            const auto reqAmount = req.payment() / myEntry_->price;
            if (remainAmount <= reqAmount) {
                const auto sales = req.trade(remainAmount);
                depositFn(sales);
                return;
            }
            const auto sales = req.trade(reqAmount);
            depositFn(sales);
            remainAmount -= reqAmount;
        }
    }

    [[nodiscard]] auto isPosting() const noexcept -> bool { return myEntry_.has_value(); }

    [[nodiscard]] auto packRequest() noexcept -> std::span<RefWrap<RequestT>> {
        static thread_local auto refs = std::vector<RefWrap<RequestT>>{};
        refs.clear();
        auto requests = myEntry_->requests();
        refs.reserve(requests.size());
        for (auto& req : requests) refs.emplace_back(std::ref(req));
        return refs;
    }

    void performFullTrade(DepositFn auto&& depositFn) noexcept {
        for (auto& request : myEntry_->requests()) {
            const auto tradeAmount = request.payment() / myEntry_->price;
            const auto sales       = request.trade(tradeAmount);
            depositFn(sales);
        }
    }

    Ledger                  ledger_;
    std::optional<EntryT&>  myEntry_{std::nullopt};
    mutable RandomGenerator rng_;
};
}  // namespace abm::base_goods::supplier