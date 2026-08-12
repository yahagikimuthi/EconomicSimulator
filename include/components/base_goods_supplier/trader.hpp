#pragma once

#include <tbb/concurrent_vector.h>
#include <functional>
#include <optional>
#include <pcg_random.hpp>
#include <ranges>
#include <type_traits>
#include <utility>
#include <vector>

#include "core/base.hpp"
#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "world/message.hpp"

namespace abm::base_goods::supplier {
template <Market SupplyGoodsType>
class Trader {
    using Entry = std::conditional_t<
        SupplyGoodsType == Market::consumerGoods,
        ConsumerGoodsEntry,
        ProductionGoodsEntry>;
    using Request = std::conditional_t<
        SupplyGoodsType == Market::consumerGoods,
        ConsumerGoodsRequest,
        ProductionGoodsRequest>;

  public:
    void trade() {
        if (not isPosting_) return;
        auto&               requestBox = myEntry_->requestBox;
        const GoodsQuantity totalDemand{calcTotalDemand(requestBox)};
        if (totalDemand == GoodsQuantity{0.0}) return;
        const bool          isExcessDemand{totalDemand > ledger_.inventory};
        const GoodsQuantity salesAmount{std::min(ledger_.inventory, totalDemand)};
        isExcessDemand ? performRationedTrade(myEntry_->supply, rng_, requestBox)
                       : performFullTrade(requestBox);
        ledger_.totalDemand += totalDemand;
        ledger_.currentSales += myEntry_->price * salesAmount;
        ledger_.inventory -= salesAmount;
    }

    auto inventory() const -> GoodsQuantity POST(inv : inv >= GoodsQuantity{0.0}) {
        return ledger_.inventory;
    }

    auto sales() const -> Money POST(sales : sales >= Money{0.0}) { return ledger_.currentSales; }

    auto totalDemand() const -> GoodsQuantity POST(demand : demand >= GoodsQuantity{0.0}) {
        return ledger_.totalDemand;
    }

    void endStep() { myEntry_.reset(), isPosting_ = false, ledger_.reset(); }

  protected:
    Trader(const pcg32 rng) : rng_{rng} {}

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

    static void shuffleIdx(
        tbb::concurrent_vector<Request>&              requestBox,
        std::vector<std::reference_wrapper<Request>>& requests,
        pcg32&                                        rng
    ) {
        requests.clear();
        for (Request& request : requestBox) requests.emplace_back(std::ref(request));
        std::ranges::shuffle(requests, rng);
    }

    static void performRationedTrade(
        const GoodsQuantity supply, pcg32& rng, tbb::concurrent_vector<Request>& requestBox
    ) {
        static thread_local std::vector<std::reference_wrapper<Request>> requests;
        shuffleIdx(requestBox, requests, rng);

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

    pcg32                 rng_;
    std::optional<Entry&> myEntry_{std::nullopt};
    bool                  isPosting_{false};

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
}  // namespace abm::base_goods::supplier