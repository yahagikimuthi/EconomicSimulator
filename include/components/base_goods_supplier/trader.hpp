#pragma once

#include <tbb/concurrent_vector.h>
#include <functional>
#include <optional>
#include <pcg_random.hpp>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

#include "components/base_goods_supplier/common.hpp"
#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "util.hpp"
#include "world/message.hpp"

namespace abm::base_goods::supplier {
class LedgerManager {
  public:
    [[nodiscard]] LedgerManager() = default;

    void makeNewPage(const GoodsQuantity supply) { supply_ = supply; }

    void add(const Price price, GoodsQuantity salesAmount) {
        inventory_ -= salesAmount;
        currentSales_ += price * salesAmount;
        totalDemand_ += salesAmount;
    }

    void reset() {
        supply_       = GoodsQuantity{0.0};
        inventory_    = GoodsQuantity{0.0};
        currentSales_ = Money{0.0};
        totalDemand_  = GoodsQuantity{0.0};
    }

    [[nodiscard]] auto isExcessDemand(const GoodsQuantity demand) const -> bool {
        return demand >= inventory_;
    }

    [[nodiscard]] auto salesAmount(const GoodsQuantity demand) -> GoodsQuantity {
        return isExcessDemand(demand) ? inventory_ : demand;
    }

    [[nodiscard]] auto tradingResult() const -> TradeResult {
        return {
            .supply      = supply_,
            .soldAmount  = supply_ - inventory_,
            .totalDemand = totalDemand_,
            .sales       = currentSales_
        };
    }

  private:
    GoodsQuantity supply_{0.0};
    GoodsQuantity inventory_{0.0};
    Money         currentSales_{0.0};
    GoodsQuantity totalDemand_{0.0};
};

template <Market SupplyGoodsType>
    requires(SupplyGoodsType != Market::labor)
class Trader {
    using Entry = std::conditional_t<
        SupplyGoodsType == Market::consumerGoods,
        ConsumerGoodsEntry,
        ProductionGoodsEntry>;
    using Request = std::conditional_t<
        SupplyGoodsType == Market::consumerGoods,
        ConsumerGoodsRequest,
        ProductionGoodsRequest>;

    template <typename T>
    using RefWrap = std::reference_wrapper<T>;

  public:
    void trade() {
        if (not isPosting_) return;
        auto&               requestBox = myEntry_->requestBox;
        const GoodsQuantity totalDemand{calcTotalDemand(requestBox)};
        if (totalDemand == GoodsQuantity{0.0}) return;
        const GoodsQuantity salesAmount{ledgerManager_.salesAmount(totalDemand)};
        ledgerManager_.isExcessDemand(totalDemand)
            ? performRationedTrade(myEntry_->supply, requestBox)
            : performFullTrade(requestBox);
        ledgerManager_.add(myEntry_->price, salesAmount);
    }

    [[nodiscard]] auto tradingResult() const -> TradeResult {
        return ledgerManager_.tradingResult();
    }

    void endStep() { myEntry_.reset(), isPosting_ = false, ledgerManager_.reset(); }

  protected:
    Trader(const RandomGenerator rng) : rng_{rng} {}

    mutable RandomGenerator rng_;
    std::optional<Entry&>   myEntry_{std::nullopt};
    LedgerManager           ledgerManager_;
    bool                    isPosting_{false};

  private:
    static auto calcTotalDemand(const tbb::concurrent_vector<Request>& requestBox
    ) -> GoodsQuantity {
        const GoodsQuantity demand{std::ranges::fold_left(
            requestBox | std::ranges::views::transform([](const Request& req) -> double {
                return req.amount.value();
            }),
            0.0,
            std::plus<>{}
        )};
        ASSERT(demand >= GoodsQuantity{0.0});
        return demand;
    }

    void shuffleIdx(
        tbb::concurrent_vector<Request>& requestBox, std::vector<RefWrap<Request>>& requests
    ) const {
        requests.clear();
        for (Request& request : requestBox) requests.emplace_back(std::ref(request));
        rng_.shuffle(requests);
    }

    void performRationedTrade(
        const GoodsQuantity supply, tbb::concurrent_vector<Request>& requestBox
    ) const {
        static thread_local std::vector<RefWrap<Request>> requests;
        shuffleIdx(requestBox, requests);

        GoodsQuantity remainAmount{supply};
        for (auto requestRef : requests) {
            auto&               request = requestRef.get();
            const GoodsQuantity requestAmount{request.amount};
            if (remainAmount <= requestAmount) {
                request.tradeAmount = remainAmount;
                return;
            }
            request.tradeAmount = requestAmount;
            remainAmount -= requestAmount;
        }

        ASSERT(false && "runtime error");
        std::unreachable();
    }

    static void performFullTrade(tbb::concurrent_vector<Request>& requestBox) {
        for (auto& request : requestBox) request.tradeAmount = request.amount;
    }
};
}  // namespace abm::base_goods::supplier