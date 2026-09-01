#pragma once

#include <optional>
#include <ranges>
#include <span>
#include <vector>

#include "components/base_goods_supplier/common.hpp"
#include "components/capital_supplier/ledger.hpp"
#include "components/finance/firm_finance.hpp"
#include "others/util.hpp"
#include "values/common.hpp"
#include "values/goods.hpp"
#include "world/base_goods.hpp"

namespace abm::capital::supplier {
class Trader final {
    using TradePlan   = base_goods::supplier::TradePlan;
    using TradeResult = base_goods::supplier::TradeResult;

  public:
    explicit Trader(RandomGenerator& masterRng) noexcept
        : rng_{pcg32{masterRng.makeUint64(), masterRng.makeUint64()}} {}

    void post(const AgentID id, const TradePlan& plan, Market& market) noexcept {
        ASSERT(plan.supply.isZeroOrMore());
        isActive_ = true;
        if (plan.supply.isZero()) return;
        myEntry_ = market.entry(id, plan.price, plan.supply);
        ledger_.makeNewPage(plan.supply);
    }

    [[nodiscard]] auto trade(FirmFinance& finance) noexcept -> std::optional<TradeResult> {
        if (not isPosting()) return std::nullopt;
        const auto demand = calcTotalDemand();
        if (demand.isZero()) return std::nullopt;
        const auto tradeAmount    = ledger_.tradableAmount(demand);
        const auto isExcessDemand = ledger_.isExcessDemand(demand);
        isExcessDemand ? performRationedTrade(finance) : performFullTrade(finance);
        return ledger_.publishResult(
            {.price = myEntry_->price, .demand = demand, .salesAmount = tradeAmount}
        );
    }

    void reset() noexcept {
        myEntry_.reset();
        isActive_ = false;
    }

  private:
    [[nodiscard]] auto calcTotalDemand() const noexcept -> GoodsQuantity {
        const auto requests = myEntry_->requests();
        return std::ranges::fold_left(
            requests | std::views::transform([&](const Request& req) noexcept -> GoodsQuantity {
                return req.payment() / myEntry_->price;
            }),
            GoodsQuantity{0.0},
            std::plus{}
        );
    }

    void performRationedTrade(FirmFinance& finance) noexcept {
        auto requests = packRequest();
        rng_.shuffle(requests);

        auto remainAmount = ledger_.inventory();
        for (auto reqRef : requests) {
            auto&      req       = reqRef.get();
            const auto reqAmount = req.payment() / myEntry_->price;
            if (remainAmount <= reqAmount) {
                const auto sales = req.trade(remainAmount);
                finance.assetPlus(sales, FirmFinance::AccountItem::Sales);
                return;
            }
            const auto sales = req.trade(reqAmount);
            finance.assetPlus(sales, FirmFinance::AccountItem::Sales);
            remainAmount -= reqAmount;
        }
    }

    [[nodiscard]] auto isPosting() const noexcept -> bool { return myEntry_.has_value(); }

    [[nodiscard]] auto packRequest() noexcept -> std::span<RefWrap<Request>> {
        static thread_local auto refs = std::vector<RefWrap<Request>>{};
        refs.clear();
        auto requests = myEntry_->requests();
        refs.reserve(requests.size());
        for (auto& req : requests) refs.emplace_back(std::ref(req));
        return refs;
    }

    void performFullTrade(FirmFinance& finance) noexcept {
        for (auto& request : myEntry_->requests()) {
            const auto tradeAmount = request.payment() / myEntry_->price;
            const auto sales       = request.trade(tradeAmount);
            finance.assetPlus(sales, FirmFinance::AccountItem::Sales);
        }
    }

    Ledger                  ledger_;
    std::optional<Entry&>   myEntry_{std::nullopt};
    mutable RandomGenerator rng_;
    bool                    isActive_{false};
};
}  // namespace abm::capital::supplier