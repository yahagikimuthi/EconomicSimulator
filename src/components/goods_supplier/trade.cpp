#include "components/goods_supplier.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cassert>
#include <functional>
#include <pcg_random.hpp>
#include <ranges>

#include "core/values/goods.hpp"
#include "world/message.hpp"

namespace {
[[nodiscard]] auto calcTotalDemand(const tbb::concurrent_vector<world::GoodsRequest>& requestBox
) -> GoodsQuantity {
    const GoodsQuantity demand{std::ranges::fold_left(
        requestBox | std::ranges::views::transform([](const world::GoodsRequest& req) -> double {
            return req.amount.value();
        }),
        0.0,
        std::plus<>{}
    )};
    assert(demand >= GoodsQuantity{0.0} && "total demand is required >= 0");
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
    const GoodsQuantity supply, pcg32& rng, tbb::concurrent_vector<world::GoodsRequest>& requestBox
) {
    static thread_local std::vector<std::reference_wrapper<world::GoodsRequest>> requests;
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

void performFullTrade(tbb::concurrent_vector<world::GoodsRequest>& requestBox) {
    for (auto& request : requestBox) {
        request.tradeAmount = request.amount;
    }
}
}  // namespace

namespace goods::supplier {
void Trader::trade() {
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
}  // namespace goods::supplier