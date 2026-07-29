#include "components/goods_supplier.hpp"

#include <tbb/concurrent_vector.h>
#include <algorithm>
#include <cassert>
#include <functional>
#include <pcg_random.hpp>
#include <ranges>

#include "world/message.hpp"

namespace {
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
}  // namespace

namespace goods_supplier {
void Trader::trade() {
    if (not isPosting) return;
    auto&        requestBox = myEntry_->requestBox;
    const double totalDemand{calcTotalDemand(requestBox)};
    if (totalDemand == 0.0) return;
    const bool   isExcessDemand{totalDemand > myEntry_->supply};
    const double salesAmount{std::min(myEntry_->supply, totalDemand)};
    isExcessDemand ? performRationedTrade(myEntry_->supply, rng_, requestBox)
                   : performFullTrade(requestBox);
    ledger_.totalDemand += totalDemand;
    ledger_.currentSales += totalDemand * myEntry_->price;
    ledger_.inventory -= salesAmount;
}
}  // namespace goods_supplier