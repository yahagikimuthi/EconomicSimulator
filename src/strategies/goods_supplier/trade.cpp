#include "strategies/goods_supplier/trade.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cassert>
#include <functional>
#include <pcg_random.hpp>
#include <ranges>

#include "world/message.hpp"

namespace goods_supplier::internal {
[[nodiscard]] auto calcTotalDemand(const tbb::concurrent_vector<world::GoodsRequest>& requestBox
) -> double {
    const double demand{std::ranges::fold_left(
        requestBox | std::ranges::views::transform(&world::GoodsRequest::amount), 0.0, std::plus<>{}
    )};
    assert(demand >= 0.0 && "total demand is required >= 0");
    return demand;
}

void shuffleIdx(
    tbb::concurrent_vector<world::GoodsRequest>&              requestBox,
    std::vector<std::reference_wrapper<world::GoodsRequest>>& requests,
    pcg32&                                                    rng
) {
    requests.clear();
    for (world::GoodsRequest& request : requestBox) requests.emplace_back(std::ref(request));
    std::ranges::shuffle(requests, rng);
}

void performRationedTrade(
    const double supply, pcg32& rng, tbb::concurrent_vector<world::GoodsRequest>& requestBox
) {
    static thread_local std::vector<std::reference_wrapper<world::GoodsRequest>> requests;
    shuffleIdx(requestBox, requests, rng);

    double remainAmount{supply};
    for (auto requestRef : requests) {
        auto&        request = requestRef.get();
        const double requestAmount{request.amount};
        if (remainAmount <= requestAmount) {
            request.tradeAmount = remainAmount;
            return;
        }
        request.tradeAmount = requestAmount;
        remainAmount -= requestAmount;
    }

    assert(false && "runtime error");
    std::unreachable();
}

void performFullTrade(tbb::concurrent_vector<world::GoodsRequest>& requestBox) {
    for (auto& request : requestBox) {
        request.tradeAmount = request.amount;
    }
}
}  // namespace goods_supplier::internal

namespace goods_supplier {
void trade(TradeView view) {
    if (not view.isPosting()) return;
    auto& myEntry    = view.myEntry();
    auto& requestBox = myEntry.requestBox;

    const double totalDemand{internal::calcTotalDemand(requestBox)};
    if (totalDemand == 0.0) return;
    const bool   isExcessDemand{totalDemand > myEntry.supply};
    const double salesAmount{std::min(myEntry.supply, totalDemand)};
    isExcessDemand ? internal::performRationedTrade(myEntry.supply, view.rng(), requestBox)
                   : internal::performFullTrade(requestBox);
    view.inventoryMinus(salesAmount);
    view.salesPlus(salesAmount * myEntry.price);
    view.totalDemandPlus(totalDemand);
}
}  // namespace goods_supplier