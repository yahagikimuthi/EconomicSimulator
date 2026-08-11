#pragma once

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <functional>
#include <optional>
#include <pcg_random.hpp>
#include <ranges>
#include <type_traits>

#include "core/values/common.hpp"
#include "core/values/goods.hpp"
#include "world/message.hpp"

namespace abm::base_goods::supplier {
template <typename T>
concept EntryType = (std::same_as<T, ConsumerGoodsEntry> or std::same_as<T, ProductionGoodsEntry>);

template <typename T>
concept RequestType =
    (std::same_as<T, ConsumerGoodsRequest>) or std::same_as<T, ProductionGoodsRequest>;

template <RequestType T>
[[nodiscard]] inline auto calcTotalDemand(const tbb::concurrent_vector<T>& requestBox
) -> GoodsQuantity {
    const GoodsQuantity demand{std::ranges::fold_left(
        requestBox | std::ranges::views::transform([](const T& req) -> double {
            return req.amount.value();
        }),
        0.0,
        std::plus<>{}
    )};
    assert(demand >= GoodsQuantity{0.0} && "total demand is required >= 0");
    return demand;
}

template <RequestType T>
void inline shuffleIdx(
    tbb::concurrent_vector<T>&              requestBox,
    std::vector<std::reference_wrapper<T>>& requests,
    pcg32&                                  rng
) {
    requests.clear();
    for (T& request : requestBox) requests.emplace_back(std::ref(request));
    std::ranges::shuffle(requests, rng);
}

template <RequestType T>
void inline performRationedTrade(
    const GoodsQuantity supply, pcg32& rng, tbb::concurrent_vector<T>& requestBox
) {
    static thread_local std::vector<std::reference_wrapper<T>> requests;
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

template <RequestType T>
void inline performFullTrade(tbb::concurrent_vector<T>& requestBox) {
    for (auto& request : requestBox) request.tradeAmount = request.amount;
}

template <Market SupplyGoodsType>
class Trader {
    using Entry = std::conditional<
        SupplyGoodsType == Market::consumerGoods,
        ConsumerGoodsEntry,
        ProductionGoodsEntry>;
    using Request = std::conditional<
        SupplyGoodsType == Market::consumerGoods,
        ConsumerGoodsRequest,
        ProductionGoodsRequest>;

  public:
    Trader(const pcg32 rng) : rng_{rng} {}

    void post(
        const GoodsQuantity supply, const Price pricePlan, tbb::concurrent_vector<Entry>& entryBox
    ) {
        if (supply == GoodsQuantity{0.0}) return;
        isPosting_        = true;
        ledger_.inventory = supply;
        auto it{entryBox.emplace_back(pricePlan, supply)};
        myEntry_ = *it;
    }

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

  private:
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